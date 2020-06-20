#ifndef LINUXRTM_H
#define LINUXRTM_H

typedef int SOCKET;
typedef void *HANDLE;
typedef void *LPVOID;
typedef int SIZE_T;
typedef char bool;

#define fwprintf fprintf
#define ioctlsocket ioctl

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define WSAEWOULDBLOCK EWOULDBLOCK
#define SD_BOTH SHUT_RDWR
#define _wtoi atoi
#define GetCurrentThreadId pthread_self


#define MAX_THREAD_NAMES 1024
#define MAX_FILE_PATH 4096



void logg_begin();
void logg_end();
void logg(const char_t* pFormat, ...);
void logt(const char* pFormat, ...);
int set_thread_priority(int id, int priority);
void kill_ps(int id);
int get_ps_path(int pid, char_t* path, int sz);
void kill_ps(int id);
int start_ps(char_t* path);
void kill_thread(int tid);
void suspend_thread(int tid);
void resume_thread(int tid);
int set_thread_priority(int id, int priority);
int get_thread_priority(int id);
int GetLastError();
void dieRtm();

char splitString(char_t* s, int s_len, char_t res[MAX_TOKENS][MAX_TOKEN_SZ], int ressz, int resisz, char_t *delimiter, int numdelimiters);
//unsigned long int strtoul ( const char * str, char ** endptr, int base );
//int atoi ( const char * str );
//void strtime(char_t *dtw, int dtsz);
void Sleep(int ms);
void getSystemInfo(MY_SYSTEM_INFO *sysinfo);
unsigned long timeElapsed();
void getMemoryInfo(MY_MEM_INFO *memstatus);

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>

#include "comm.h"

void sigregister();
bool expandList(COMM_MESG **msg, DWORD *pidsz, DWORD *tidsz, DWORD n, bool isps);
bool getPsStats(const char *path, bool isps, DWORD ppid, COMM_MESG **msg, DWORD *pidsz, DWORD *tidsz, DWORD *pn, DWORD *tn);


#endif//LINUXRTM
