#ifndef COMM_H
#define COMM_H

#ifdef windows
#include <winsock2.h>
#endif
#include "../common/types.h"

/*
To fix Unresolved external __security_cookie
Code Generation: Buffer security check (disable it)
*/

#define SERVER_PORT 8291
#define SERVER_DISCOVERY_PORT 8292
//#define SERVER_IP TXT("172.17.1.1")
//#define SERVER_IP TXT("192.168.1.32")
#define SERVER_IP TXT("127.0.0.1")
//#define SERVER_IP TXT("172.17.210.174")
#define SERVER_PORT_S TXT("8291")
#define SNAP_INTERVAL TXT("100")
#define MIN_SNAP_INTERVAL 100
#define MAX_SNAP_INTERVAL 1000*60*60*24
#define WAIT_ON_SOCKET_TIMEOUT_S 5
#define MAX_CLIENTS 10
#define RTMON_VERSION 710
#define MAX_RFILE_PATH 1024
#define MAX_CLIENT_RD_BUFFER_SZ MAX_RFILE_PATH*sizeof(char_t)+512
#define MAX_DIR_PATH 4096
#define MAX_FILENAME_LEN 512
#define MAX_FRAMES 20
#define MAX_CORES 8

#if (_MSC_VER==1400)
typedef unsigned long long long64;
#elif defined (windows)
typedef unsigned __int64 long64;
#else
#endif

enum CLIENT_SOCKET_STATE
{
	CLIENT_SOCKET_STATE_NONE = 0,
	CLIENT_SOCKET_STATE_CONNECTED = 0x1,
	CLIENT_SOCKET_STATE_SEND_SNAPINFO = 0x2,
	CLIENT_SOCKET_STATE_SEND_MEMINFO = 0x4,
};

typedef struct CLIENT_
{
	unsigned char rdbuffer[MAX_CLIENT_RD_BUFFER_SZ];
	int rdbuffer_size;
	int rdbuffer_head;
	int rdbuffer_tail;
	SOCKET so;
	FILETIME last_activity;
	unsigned char state;
}
CLIENT;

enum COMM_CMD
{
   COMM_CMD_INIT_SETTINGS,
   COMM_CMD_SYSINFO,
   COMM_CMD_PDATA,
   COMM_CMD_TDATA,
   COMM_CMD_TNAME,
   COMM_CMD_SNAPINFO,
   COMM_CMD_CLOSE,
   COMM_CMD_NOOP,
   COMM_CMD_CLIENTS_MAXED,
   COMM_CMD_GET_SNAPSHOT,
   COMM_CMD_GET_MEMINFO,
   COMM_CMD_GET_PROCESS_PATH,
   COMM_CMD_KILL_PROCESS,
   COMM_CMD_START_PROCESS,
   COMM_CMD_CLONE_PROCESS,
   COMM_CMD_RESTART_PROCESS,
   COMM_CMD_KILL_THREAD,
   COMM_CMD_SUSPEND_THREAD,
   COMM_CMD_RESUME_THREAD,
   COMM_CMD_THREAD_PRIORITY,
   COMM_CMD_DEVICE_SHUTDOWN,
   COMM_CMD_DEVICE_SUSPEND,
   COMM_CMD_DEVICE_RESTART,
   COMM_CMD_SERVER_RESTART,
   COMM_CMD_CAPTURE_ON_DEV,
   COMM_CMD_KDUMP_DIR,
   COMM_CMD_FILE,
   COMM_CMD_DEBUG_NEW_PROCESS,
   COMM_CMD_DEBUG_ACTIVE_PROCESS,
   COMM_CMD_CRASH_INFO,
   COMM_CMD_SET_PROCESS_AFFINITY,
   COMM_CMD_SET_THREAD_AFFINITY,
   COMM_CMD_TOTAL
};

typedef struct COMM_INIT_SETTINGS_
{
   int priority;
   DWORD snap_interval;
   int capture_on_dev;
   int logging;

}COMM_INIT_SETTINGS;

typedef struct COMM_MESG_
{
   int cmd;
   int sz;
   union
   {
      int numelm;
      int id;
      int show;
      int get;
      int set;
   };
}COMM_MESG;

typedef struct EXECOBJ_DATA_
{
    DWORD pid;
    DWORD tid;//if non zero, then this is thread data
    DWORD mem;//only valid if ps, same as rss field in /proc/stat
#if 1//def linux
   unsigned long cpuTimeSpent;
   unsigned long priority;
   unsigned long nice;
   unsigned long rt_priority;
   unsigned long policy;
   unsigned long ft;
   unsigned long core;
#endif
    char2_t name[64];
}
EXECOBJ_DATA;

typedef struct THREAD_NAME_
{
   DWORD tid;
   char2_t name[64];
}
THREAD_NAME;

typedef struct THREAD_PRIORITY_
{
   COMM_MESG msg;
   DWORD priority;
}
THREAD_PRIORITY;

typedef struct PROCESS_START_
{
   COMM_MESG msg;
   DWORD rqstid;
   DWORD pid;
   char_t name[MAX_RFILE_PATH];
}
PROCESS_START;

typedef struct MY_POWER_INFO_
{
	float battery_percent;
	int ac;
	int battery;
}
MY_POWER_INFO;

typedef struct COMM_SNAPINFO_
{
	FILETIME ft;//tick count in ms
	long64 now;//millisecond
	MY_MEM_INFO memstatus;
	struct tm dt;
	MY_POWER_INFO power;
	DWORD idle[MAX_CORES];//array of idle times for each core in ms
}COMM_SNAPINFO;

typedef struct CAPTURE_ON_DEV
{
   COMM_MESG msg;
   DWORD priority;
   int capture_on_dev;
   DWORD interval_ms;
}
CAPTURE_ON_DEV;

typedef struct KDUMP_DIR_
{
   COMM_MESG msg;
   char_t name[1];
}
KDUMP_DIR;

typedef struct FILE_DATA_
{
   COMM_MESG msg;
   char_t data[1];
}
FILE_DATA;

typedef struct SET_CORE_
{
	COMM_MESG msg;
	int affinity;
}
SET_CORE;

typedef struct DEBUG_PROCESS_ACK_
{
   COMM_MESG msg;
   DWORD rqstid;
   DWORD pid;
}
DEBUG_PROCESS_ACK;

#ifndef STACKSNAP_FAIL_IF_INCOMPLETE
#define STACKSNAP_FAIL_IF_INCOMPLETE     1
#define STACKSNAP_EXTENDED_INFO          2
#define STACKSNAP_INPROC_ONLY            4
#define STACKSNAP_RETURN_FRAMES_ON_ERROR 8
typedef struct _CallSnapshotEx {
  DWORD dwReturnAddr;
  DWORD dwFramePtr;
  DWORD dwCurProc;
  DWORD dwParams[4];
} CallSnapshotEx;//This is the call frame
#endif//STACKSNAP_FAIL_IF_INCOMPLETE

#ifdef windows
typedef struct CRASH_INFO_
{
   COMM_MESG msg;
   DWORD rqstid;
   DEBUG_EVENT debug_event;
   struct CRASH_INFO_ *next;
   CallSnapshotEx frame[1];
}
CRASH_INFO;
#endif

typedef struct DISCOVERY_PING_
{
	char program[8];
	DWORD version;
}
DISCOVERY_PING;

typedef struct DISCOVERY_PONG_
{
	char program[8];
	DWORD version;
	unsigned short server_port;
	__int64 instance;
	DWORD connected_flag;
	char_t name[32];
}
DISCOVERY_PONG;


int winsock_init();
void winsock_end();

SOCKET socket_server_init(short port, char block);
void socket_server_end(SOCKET s, char block);

SOCKET socket_accept(SOCKET SocketFD, char block);

int socket_read(SOCKET s, unsigned char *buf, int len, char block);
int socket_write(SOCKET s, const unsigned char *buf, int len, char block);

int socket_write_clients(const unsigned char *buf, int len, char state, char block);
void socket_write_client(SOCKET s, const unsigned char *buf, int len, char block);

void socket_set_client_state(SOCKET s, unsigned char state);
void socket_unset_client_state(SOCKET s, unsigned char state);
void socket_unset_clients_state(unsigned char state);
unsigned char socket_is_any_client_state(unsigned char state);

void socket_close_client(SOCKET ConnectFD);

int socket_client_connect(SOCKET SocketFD, const char* ip, short port, int *error);
int socket_client_connect2(SOCKET SocketFD, ULONG ip, short port, int *error);

int socket_nonblocking(SOCKET SocketFD);
int socket_blocking(SOCKET SocketFD);

int socket_set_broadcast(SOCKET SocketFD);

SOCKET socket_client_init();

typedef int (*PROCESS_CLIENT_REQUEST)(SOCKET s, unsigned char *buffer, int *head, int tail);
void socket_read_client(SOCKET client_so, PROCESS_CLIENT_REQUEST process_client_request, char block);
int socket_read_clienti(int i, PROCESS_CLIENT_REQUEST process_client_request, char block);
int socket_find_client(SOCKET client_so);
int socket_ready(CLIENT **clients, fd_set *readfds, int timeout_s, int timeout_ms);
int socket_remove_overdue_clients(char block);

int GetNumClients();


#endif//COMM_H
