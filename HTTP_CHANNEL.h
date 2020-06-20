#pragma once

#include "stdafx.h"
#include "../ds/ds.h"
#include "../common/util.h"
extern "C"{
#include "comm.h"
}

enum HTTP_CHANNEL_STATE
{
	HTTP_CHANNEL_STATE_NONE,
	HTTP_CHANNEL_STATE_CONNECT,
	HTTP_CHANNEL_STATE_CONNECTING,
	HTTP_CHANNEL_STATE_CONNECTED,
	HTTP_CHANNEL_STATE_WAITING,
	HTTP_CHANNEL_STATE_DISCONNECTED,
};

enum HTTP_CHANNEL_CODE
{
	HTTP_CHANNEL_CODE_OK,
	HTTP_CHANNEL_CODE_OK_BUT_NO_PLAY=-1,
	HTTP_CHANNEL_CODE_SESSION_EXPIRED=-2,
	HTTP_CHANNEL_CODE_PASSWORD_MISMATCH=-3,
	HTTP_CHANNEL_CODE_USER_NOT_FOUND=-4,
	HTTP_CHANNEL_CODE_REQUEST_INVALID=-5,
	HTTP_CHANNEL_CODE_RESPONSE_INVALID=-6,
	HTTP_CHANNEL_CONNECT_FAILED=-7,
};

#if 0
typedef struct TOKEN
{
	union
	{
		struct 
		{
			unsigned long field1 : 6;//low order field
			unsigned long field2 : 6;
			unsigned long field3 : 6;
			unsigned long field4 : 6;
			unsigned long blank	: 8;//high order field
		}
		bitfield;

		struct
		{
			unsigned long byte1 : 8;//low order byte
			unsigned long byte2 : 8;
			unsigned long byte3 : 8;
			unsigned long blank : 8;//high order byte
		}
		byte;

		long dword;
	};
}
TOKEN;
#endif

class HTTP_CHANNEL
{
	SOCKET m_socket;
	char *m_buffer;
	char *m_content;
	int m_content_sz;
	int m_buffer_sz;
	int m_buffer_pos;
	int m_state;
	STRING m_host;
	STRING m_uri;
	unsigned short m_port;
	static char m_key[];

	void close();
	bool send(const STRING &uri, char* rqst, int rqst_sz);
	int recv(char* buffer, int buffer_sz, char **resp, int *resp_sz);
	int base64Encode(char* in_buffer, int in_num_bytes_to_encode,  int in_buffer_sz, char* out_buffer, int in_out_buffer_sz);
public:
	HTTP_CHANNEL(void);
	virtual ~HTTP_CHANNEL(void);

	bool init(const STRING &host, unsigned short port);
	int request(const STRING &uri, char* rqst, int rqst_sz, char** resp, int *resp_sz);
};
