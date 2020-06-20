#include "HTTP_CHANNEL.h"
#include "../common/util.h"

#define LINE_LEN 1024
#define HTTP_OK "http/1.1 200 ok"
#define CONTENT_ENCODING "content-transfer-encoding: binary"
#define TRANSFER_ENCODING "transfer-encoding: chunked"
#define CONTENT_LENGTH "content-length: "
#define CONTENT_TYPE "content-type: "
#define MAX_HTTP_CHANNEL_RECV_DATA_SIZE 1024*1024

#ifdef UNICODE
#undef atoi
#endif

HTTP_CHANNEL::HTTP_CHANNEL(void)
: m_socket(0)
, m_state(HTTP_CHANNEL_STATE_NONE)
, m_buffer(0)
, m_buffer_sz(0)
, m_buffer_pos(0)
, m_port(80)
{
}

HTTP_CHANNEL::~HTTP_CHANNEL(void)
{
}

bool HTTP_CHANNEL::init(const STRING &host, unsigned short port)
{
	m_buffer_sz=MAX_HTTP_CHANNEL_RECV_DATA_SIZE;
	m_buffer=new char[m_buffer_sz];
	if(!m_buffer)
		return false;
	m_buffer_pos=0;
	m_state=HTTP_CHANNEL_STATE_CONNECT;
	m_host = host;
	m_port = port;

	return true;
}

int HTTP_CHANNEL::recv(char* buffer, int buffer_sz, char **resp, int *resp_sz)
{
	int ret = 0;
	int i,j;
	int chunk_size=0;
	bool http_ok=false;
	bool binary_content=false;
	bool chunked=false;
	bool octet_stream=false;
	char *line = new char [LINE_LEN];
	if(!line)
		return 0;

	m_content_sz=0;
	m_content=0;
	j=0;

	if(buffer_sz>0 && buffer && *buffer)
	{
		for(i=0; i<buffer_sz-1; i++)
		{
			if(buffer[i]=='\r' && buffer[i+1]=='\n')
			{//parse line
				if(!http_ok && memcmp(line, HTTP_OK, lstrlenA(HTTP_OK))==0)
					http_ok=true;
				else if(http_ok && memcmp(line, CONTENT_TYPE, lstrlenA(CONTENT_TYPE))==0)
					octet_stream=true;
				else if(http_ok && memcmp(line, TRANSFER_ENCODING, lstrlenA(TRANSFER_ENCODING))==0)
					chunked=true;
				else if(http_ok && memcmp(line, CONTENT_ENCODING, lstrlenA(CONTENT_ENCODING))==0)
					binary_content=true;
				else if(http_ok && memcmp(line, CONTENT_LENGTH, lstrlenA(CONTENT_LENGTH))==0)
					m_content_sz=atoi(line+lstrlenA(CONTENT_LENGTH));
				else if(http_ok && binary_content && octet_stream && m_content_sz>0 && j==0 && (!chunked || chunk_size>0))
				{
					m_content = m_buffer+i+2;
					break;
				}
				memset(line, 0, LINE_LEN);
				j=0;
			}
			else if(http_ok && binary_content && octet_stream && m_content_sz>0 && j==0 && chunked && !chunk_size)
			{
				i+=3;
				int k=0;
				char d=0;
				
				while(1)
				{
					if(buffer[i+k]<'g' && buffer[i+k]>='a')
						d=0xa + buffer[i] - 'a';
					else if(buffer[i+k]<='9' && buffer[i+k]>='0')
						d=buffer[i+k] - '0';
					else 
						break;
					chunk_size <<= (k*4);
					chunk_size |= d;
					k++;
				}
				if(k>0)
 					i+=k-1;
			}
			else if(i<LINE_LEN)
			{
				if(buffer[i]!='\n')
					line[j++]=tolower(buffer[i]);
			}
		}
	}
	if(line)
		delete [] line;
	if(m_content && m_content_sz>0)
	{
		*resp = m_content;
		*resp_sz = m_content_sz;
		ret = 1;
	}
	return ret;
}

void HTTP_CHANNEL::close()
{
	m_state=HTTP_CHANNEL_STATE_DISCONNECTED;
	if(m_socket)
	{
		socket_close_client(m_socket);
		m_socket=0;
	}
}
//http://www.faqs.org/rfcs/rfc1426.html
//http://www.faqs.org/rfcs/rfc1341.html
//http://www.oac.uci.edu/indiv/ehood/MIME/rfc2183.txt
//http://www.cis.ohio-state.edu/htbin/rfc/rfc2045.html
//http://www.faqs.org/rfcs/rfc1521.html

char HTTP_CHANNEL::m_key[] = {'A', 
	'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 
	'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 
	'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 
	'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 
	'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', 
	'9', '+', '/', '='
};

#if 0
int HTTP_CHANNEL::base64Encode(char* in_buffer, int in_num_bytes_to_encode,  int in_buffer_sz, char* out_buffer, int in_out_buffer_sz)
{
	unsigned long	i;
	unsigned long	j;
	unsigned char*	pFileIn=0;
	unsigned char*	pFileOut;
	unsigned long	padding=0;
	unsigned long	encodedsize=0;
	unsigned long	paddedsize=0;
	unsigned long	remainder=0;
	unsigned long	numbytesforcrlfs=0;
	TOKEN			token={0};

	//total length must be a multiple of 3 bytes (24 bits)

	remainder	= (in_num_bytes_to_encode % 3);
	padding		= remainder ? 3 - remainder : 0;
	paddedsize	= in_num_bytes_to_encode + padding;

	if(paddedsize > (unsigned int)in_buffer_sz)
		return 0;
	for(i=0; i<padding; i++)
		in_buffer[in_num_bytes_to_encode+i]=0;

	//in encoded data, an 8 bit character corresponds to 6 bits in original data
	encodedsize	= paddedsize * 8 / 6;
	//after every 76 characters, add 2 bytes for "/r/n"
	numbytesforcrlfs = 2*((encodedsize/76) + (encodedsize%76 > 0 ? 1:0));

	encodedsize += numbytesforcrlfs;

	if(!out_buffer || (unsigned int)in_out_buffer_sz<encodedsize)
		return encodedsize;

	pFileIn = (unsigned char*)in_buffer;//(unsigned char*) calloc(paddedsize, 1);
	pFileOut= (unsigned char*)out_buffer;//(unsigned char*) calloc(encodedsize, 1);
	//memcpy(pFileIn, in, in_bytes);
	memset(pFileOut+encodedsize-padding, '=', padding);

//the first bit in the stream will be the high-order bit in the first byte,
//and the eighth bit will be the low-order bit in the first byte, and so on.
//The output stream (encoded bytes)  must  be  represented  in lines  of  no 
//more than 76 characters each
	
	for(i=j=0; i < paddedsize; i+=3, j+=4)
	{
		//get the first 3 bytes
		token.byte.byte3 = pFileIn[i];
		token.byte.byte2 = pFileIn[i+1];
		token.byte.byte1 = pFileIn[i+2];

		//map to 4 characters each 6 bit
		pFileOut[j]		= token.bitfield.field4;
		pFileOut[j+1]	= token.bitfield.field3;
		pFileOut[j+2]	= token.bitfield.field2;
		pFileOut[j+3]	= token.bitfield.field1;

		//use the table to encode values
		pFileOut[j]		= m_key[ pFileOut[j] ];
		pFileOut[j+1]	= m_key[ pFileOut[j+1] ];
		pFileOut[j+2]	= m_key[ pFileOut[j+2] ];
		pFileOut[j+3]	= m_key[ pFileOut[j+3] ];

		//if you have written 76 bytes, then put a "\r\n"
		if(i%19==0)// 76/4=19 (Each time we have written 76 bytes)
		{//write carriage return
			pFileOut[j+4] = '\r';
			pFileOut[j+5] = '\n';
			j+=2;
		}

	}

	//free(pFileIn);
	return encodedsize;
}
#endif 

int HTTP_CHANNEL::request(const STRING &uri, char* rqst, int rqst_sz, char** resp, int *resp_sz)
{
	int ret = 0;//0 err, -1: continue, 1: got data from server
	int error = 0;
	int bytes_recvd=0;
	STRING err;
	STRING ip;

	switch(m_state)
	{
	case HTTP_CHANNEL_STATE_CONNECT:
		if(!gethostbyname(m_host, ip, error))
		{
			m_state = HTTP_CHANNEL_STATE_DISCONNECTED;
			break;
			//err.sprintf(L"Could not resolve host name. Error: %i", error);
		}
		m_socket = socket_client_init();
		if(!m_socket)
		{
			m_state = HTTP_CHANNEL_STATE_DISCONNECTED;
			ret = -1;
			break;
		}
		if(!socket_client_connect(m_socket, (char*)ip.ansi(), m_port, &error))
		{
			m_state = HTTP_CHANNEL_STATE_DISCONNECTED;
			break;
		}
		if(!socket_nonblocking(m_socket))
		{
			close();
			m_state = HTTP_CHANNEL_STATE_DISCONNECTED;
			break;
		}
		m_state=HTTP_CHANNEL_STATE_CONNECTED;
		ret = -1;
		break;
	case HTTP_CHANNEL_STATE_CONNECTED:
		if(!send(uri, rqst, rqst_sz))
		{
			close();
			m_state = HTTP_CHANNEL_STATE_DISCONNECTED;
			break;
		}
		m_state = HTTP_CHANNEL_STATE_WAITING;
		ret = -1;
		break;
	case HTTP_CHANNEL_STATE_WAITING:
		bytes_recvd = socket_read(m_socket, m_buffer, m_buffer_sz, false);
 		if(bytes_recvd <= 0)
		{
			if(-WSAEWOULDBLOCK != bytes_recvd)
			{
				m_state = HTTP_CHANNEL_STATE_DISCONNECTED;
				close();
				break;
			}
			ret = -1;
		}
		else if(bytes_recvd > 0)//process response
		{
			ret = recv(m_buffer, bytes_recvd, resp, resp_sz);
			if(!ret)
			{
				m_state = HTTP_CHANNEL_STATE_DISCONNECTED;
				close();
				break;
			}
		}
		m_state=HTTP_CHANNEL_STATE_CONNECTED;
		break;
	case HTTP_CHANNEL_STATE_DISCONNECTED:
		close();
		ret = 0;
		break;
	}
	return ret;
}

bool HTTP_CHANNEL::send(const STRING &uri, char* rqst, int rqst_sz)
{
	STRING header;
	int header_sz;
	char *req = 0;
	bool ret = false;

	header.set(L" ");
	header.sprintf(
		//TXT("GET %s HTTP/1.1\r\n")
		L"POST %s HTTP/1.1\r\n"
		L"Accept: text/html, */*\r\n"
		L"Accept-Language: en-us\r\n"
		//"Accept-Encoding: gzip, deflate\r\n"
		L"User-Agent: Mozilla/4.0\r\n"
		L"Host: %s\r\n"
		L"Connection: Keep-Alive\r\n"
		L"Content-Length: %i\r\n"
		//"Content-Type: application/x-www-form-urlencoded\r\n"
		L"Content-Type: application/octet-stream\r\n"
		L"\r\n",
		uri.s(), m_host.s(), rqst_sz);
	header_sz=header.size();

	req = new char[header_sz + rqst_sz];
	if(!req)
		goto Exit;

	memcpy(req, header.ansi(), header_sz);
	memcpy(req+header_sz, rqst, rqst_sz);
	
	if(SOCKET_ERROR!=socket_write(m_socket, req, header_sz + rqst_sz, false))
		m_state=HTTP_CHANNEL_STATE_WAITING;
	else
		goto Exit;

	ret=true;
Exit:	
	if(req)
		delete [] req;

	return ret;
}
