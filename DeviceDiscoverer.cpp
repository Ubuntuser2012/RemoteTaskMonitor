#include "stdafx.h"
#include "../common/types.h"
#include "../common/util.h"
#include "../ds/ds.h"
extern "C" {
#include "comm.h"
}

#include "NetworkAdapter.h"

#include "DeviceDiscoverer.h"

struct BROADCAST_IP : public OBJECT
{
	in_addr addr;
};

DeviceDiscoverer::DeviceDiscoverer()
	: m_sock(0)
{
}


DeviceDiscoverer::~DeviceDiscoverer()
{
	int i = 0;
	ITERATOR it;
	for (BROADCAST_IP* bip = (BROADCAST_IP*)m_broadcast_ips.GetFirst(it);
		i < m_broadcast_ips.GetCount();
		bip = (BROADCAST_IP*)m_broadcast_ips.GetNext(it), i++)
	{
		delete bip;
	}
	if (m_sock)
	{
		closesocket(m_sock);
		m_sock = 0;
	}
}

bool DeviceDiscoverer::Init(const DISCOVERY_PING& msg)
{
	m_broadcast_msg = msg;

	m_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sock == -1) {
		logg(TXT("error creating UDP device discovery socket"));
		return false;
	}
	socket_nonblocking(m_sock);
	socket_set_broadcast(m_sock);

	GetBroadcastIpAddresses();
	return true;
}

void DeviceDiscoverer::GetBroadcastIpAddresses()
{
	STRING err;
	NetworkAdapter na;
	na.IterateBroadcastIpAddresses([this](in_addr ba, STRING &ip) {
		BROADCAST_IP *bip = new BROADCAST_IP;
		bip->addr.S_un.S_addr = ba.S_un.S_addr;
		m_broadcast_ips.Insert(bip);
		return true;
	}, err);
}

void DeviceDiscoverer::Discover(std::function<void(sockaddr_in addr, DISCOVERY_PONG &pong)>OnDiscovered, unsigned short port)
{
	if (!m_sock)
	{
		return;
	}

	ITERATOR it;
	int i = 0;
	DWORD error = 0;
	for (BROADCAST_IP* ip = (BROADCAST_IP*)m_broadcast_ips.GetFirst(it);
		i < m_broadcast_ips.GetCount();
		ip = (BROADCAST_IP*)m_broadcast_ips.GetNext(it), i++)
	{
		struct sockaddr_in sa;
		memset(&sa, 0, sizeof sa);
		sa.sin_family = AF_INET;
		sa.sin_addr.S_un.S_addr = ip->addr.S_un.S_addr;
		sa.sin_port = htons(port);

		int bytes_sent = sendto(m_sock, (char*)&m_broadcast_msg, sizeof(m_broadcast_msg), 0, (struct sockaddr*)&sa, sizeof sa);
		error = GetLastError();
		if (bytes_sent < 0 && WSAEWOULDBLOCK != error) {
			logg(TXT("error sending UDP discovery packet, err %i\n"), error);
			break;
		}

		memset(&sa, 0, sizeof sa);
		char buffer[1024] = { 0 };
		int fromlen = sizeof sa;
		int recsize = recvfrom(m_sock, buffer, sizeof buffer, 0, (struct sockaddr*)&sa, &fromlen);
		error = GetLastError();
		if (recsize < 0 && WSAEWOULDBLOCK != error) {
			logg(TXT("error receiving UDP discovery packet, err %i\n"), error);
		}
		else if (recsize == sizeof(DISCOVERY_PONG))
		{
			DISCOVERY_PONG* pong = (DISCOVERY_PONG*)buffer;
			OnDiscovered(sa, *pong);
		}
	}
}