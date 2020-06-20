#include "../common/types.h"
#include "../common/util.h"
#include <time.h>
#include "comm.h"
#include "cemon.h"

static int m_sock;
static unsigned short m_port = 0;

bool DiscoveryServerInit(unsigned short port)
{
	struct sockaddr_in sa; 
	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(port);
	m_port = port;

	if(m_sock)
	{
		closesocket(m_sock);
		m_sock = 0;
	}

	m_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sock == -1) {
		logg(TXT("Error creating UDP device discovery server, err %i"), GetLastError());
		return 0;
	}   
	socket_nonblocking(m_sock);
	if (bind(m_sock, (struct sockaddr *)&sa, sizeof sa) == -1) {
		logg(TXT("Error binding UDP discovery socket, err %i\n"), GetLastError());
		closesocket(m_sock);
		m_sock = 0;
		return 0;
	}
	logg(TXT("discovery socket bound to port %i\n"), SERVER_DISCOVERY_PORT);
	return 1;
}

bool DiscoveryServerServe(DISCOVERY_PING *ping, DISCOVERY_PONG *pong)
{
	bool ret = 0;
	DWORD error;
	char buffer[1024] = {0};
	int recsize = 0;
	int fromlen = 0;
	struct sockaddr_in sa; 

	fromlen = sizeof sa;

	recsize = recvfrom(m_sock, (void*)buffer, sizeof buffer, 0, (struct sockaddr*)&sa, &fromlen);
	error = GetLastError();
	if (recsize < 0 && WSAEWOULDBLOCK != error) {
		logg(TXT("discovery server serve recvfrom err: %i\n"), GetLastError());
	}
	else if(recsize > 0)
	{
		char *p;
		int i = 0;
		for(p=&buffer[i]; i < recsize; i += sizeof(DISCOVERY_PING))
		{
			DISCOVERY_PING *msg = (DISCOVERY_PING*)p;
			if(memcmp(msg, ping, sizeof(DISCOVERY_PING))==0)
			{
				int bytes_sent = sendto(m_sock, (char*)pong, sizeof(DISCOVERY_PONG), 0, (struct sockaddr*)&sa, sizeof sa);
				error = GetLastError();
				if (bytes_sent < 0 && WSAEWOULDBLOCK != error) 
				{
					logg(TXT("error sending UDP discovery packet, err %i\n"), error);
				}
				ret = 1;
			}
		}
	}
	if(error == WSAECONNRESET)
	{
		DiscoveryServerInit(m_port);
	}
	return ret;
}
