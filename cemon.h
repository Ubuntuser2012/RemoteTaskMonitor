#ifndef CEMON_H
#define CEMON_H


typedef char bool;

#define timeElapsed GetTickCount
/////////////////////////////////////////////////////////////////////// 
// Platform 
// 1 Nov 01 (3.2.0.15) based on Fredrik Thelandersson, microsoft.public.pocketpc.developer, 
// 1 Nov 01, "#ifdef what??" 
// 2 May 02 major overhaul 
#if defined( FOR_PocketPC ) 
#if 0 
// Used to flag special menus, which seem to be shared by all 
three 
#if _WIN32_WCE == 420 
#define PFI_PLATFORM "Pocket PC 2003" 
#elif _WIN32_WCE == 300 
#define PFI_PLATFORM "Pocket PC 2002" 
#else 
#define PFI_PLATFORM "Pocket PC" 
#endif 
#else 
// more honest 
#define PFI_PLATFORM "Pocket PC" 
#endif 
#elif defined( FOR_PsPC ) 
#define PFI_PLATFORM "Palm-size PC" 
#elif defined( FOR_HPC ) 
#define PFI_PLATFORM "Handheld PC" 
#elif defined ( FOR_HPCPro ) 
#define PFI_PLATFORM "Handheld PC Pro" 
#elif defined( FOR_PsPC2 ) 
#define PFI_PLATFORM "Palm-size PC 1.2" 
#elif defined( FOR_PocketPC ) 
#define PFI_PLATFORM "Pocket PC" 
#elif defined( FOR_HPC2K ) 
#define PFI_PLATFORM "Handheld PC 2000" 
#elif defined( FOR_PocketPC2K ) 
#define PFI_PLATFORM "Pocket PC 2002" 
#elif defined( FOR_SSDK ) 
#define PFI_PLATFORM "Windows CE SSDK" 
#elif defined( FOR_SSDK420 ) 
#define PFI_PLATFORM "Windows CE SSDK420" 
#elif defined( FOR_DAP ) 
#define PFI_PLATFORM "DAP" 
#elif defined( FOR_PocketPC2003 ) 
#define PFI_PLATFORM "Pocket PC 2003" 
#elif defined( FOR_Desktop ) 
#define PFI_PLATFORM "Desktop" 
#elif defined( FOR_NET ) 
#define PFI_PLATFORM "Standard.Net" 
#else 
#define PFI_PLATFORM "Non_PocketPC_Platform" 
#endif //FOR_PocketPC

#if defined (MIPS ) || defined( _MIPS_ ) 
#define PFI_PROCESSOR "MIPS" 
#elif defined ( SH3 ) || defined ( _SH3_ ) 
#define PFI_PROCESSOR "SH3" 
#elif defined ( SH4 ) || defined ( _SH4_ ) 
#define PFI_PROCESSOR "SH4" 
#elif defined ( THUMB ) 
#define PFI_PROCESSOR "ARMV4T" 
#elif defined ( ARMV4 ) 
#define PFI_PROCESSOR "ARMV4" 
#elif defined ( ARM ) || defined ( _ARM_ ) 
#define PFI_PROCESSOR "ARM" 
#elif defined ( X86 ) || defined ( _X86_ ) 
#define PFI_PROCESSOR "X86" 
#elif defined ( FOR_Desktop ) 
#define PFI_PROCESSOR "X86" 
#else 
#define PFI_PROCESSOR "Unknown" 
//#error Unknown processor 
#endif 

// Consistency checks 
// Changed FOR_PSPC to FOR_PsPC 24 Mar 02 (eVT is case-sensitive) 
#ifndef PFI_PLATFORM 
//#error PFI_PLATFORM not defined 
#endif 
#ifndef PFI_PROCESSOR 
#error PFI_PROCESSOR not defined 
#endif 
// add test for FOR_PsPC2 27 May 03 
#if defined ( FOR_PocketPC ) || defined( FOR_PsPC ) || defined( FOR_PsPC2 ) || defined( FOR_PocketPC2K ) || defined( FOR_PocketPC2003 ) 
#ifndef WIN32_PLATFORM_PSPC 
#error FOR_PocketPC or FOR_PsPC defined without WIN32_PLATFORM_PSPC 
#endif 
#elif defined( WIN32_PLATFORM_PSPC ) 
////#error WIN32_PLATFORM_PSPC defined without FOR_PocketPC or FOR_PsPC 
#endif 
#if ISDEBUG 
#define DEBREL "Debug" 
#else 
#define DEBREL "Release" 
#endif 

#if defined(_WIN32_WCE) 
#if (_WIN32_WCE == 200) 
#define CEVER "Windows CE 2.00" 
#elif (_WIN32_WCE == 201) 
#define CEVER "Windows CE 2.01" 
#elif (_WIN32_WCE == 210) 
#define CEVER "Windows CE 2.10" 
#elif (_WIN32_WCE == 211) 
#define CEVER "Windows CE 2.11" 
#elif (_WIN32_WCE == 212) 
#define CEVER "Windows CE 2.12" 
#elif (_WIN32_WCE == 300) 
// Added 7 Jan 02 (3.0.0.33) 
#define CEVER "Windows CE 3.00" 
#elif (_WIN32_WCE == 400) 
// Added 6 Mar 03 
#define CEVER "Windows CE 4.00" 
#elif (_WIN32_WCE == 410) 
// Added 6 Mar 03 
#define CEVER "Windows CE 4.10" 
#elif (_WIN32_WCE == 420) 
// Added 6 Mar 03 
#define CEVER "Windows CE 4.20" 
#elif (_WIN32_WCE == 0x600) 
#define CEVER "Windows CE 6.00" 
#elif (_WIN32_WCE == 0x700) 
// Added 6 Mar 03 
#define CEVER "Windows Embedded Compact 7.00" 
#else 
#define CEVER "Unknown" 
#endif 
#else 
#define CEVER "Windows 9x" 
#endif 
#if defined(_WIN32_WCE_EMULATION) 
#define EMUL "emulated" 
#else 
#define EMUL "real" 
#endif 
#if 0 
// Can't use variables directly 
#pragma message( "CELibr: " DEBREL " build for " EMUL " " FORM " running CE version " _WIN32_WCE ) 
#else 
// 6 Jul 01 
#define FULL DEBREL " build for " EMUL " " PFI_PLATFORM " running "CEVER " on " PFI_PROCESSOR 
#pragma message( "CELibr: " FULL ) 
#endif 
#ifndef UNDER_CE 
// Not supported for Windows CE (at least 2.00) 
#pragma comment( exestr, FULL ) 
#endif 

#include "comm.h"

//#include <Psapi.h>
#include <stdio.h>
#include <Tlhelp32.h>
#ifdef _WIN32_WCE
	#include <Pm.h>
	#ifdef PKFUNCS_H
		#include <pkfuncs.h>
		#pragma comment(lib,"psapi.lib")
	#endif//PKFUNCS_H
	//#pragma comment(lib,"ws2_32.lib")
	#pragma comment(lib,"toolhelp.lib")
	#pragma comment(lib,"winsock.lib")
	#define MAX_THREAD_NAMES 1024
	#define SOCKET_READ_INTERVAL 500
	#define MAX_FILE_PATH 2048
#else//!_WIN32_WCE
	#include <Psapi.h>
	#pragma comment(lib,"ws2_32.lib")
	#pragma comment(lib,"psapi.lib")
	#define SOCKET_READ_INTERVAL 100
	#define MAX_THREAD_NAMES 1024
	#define MAX_FILE_PATH 4096
#endif//_WIN32_WCE

#ifndef _WIN32_WCE
typedef struct _SYSTEM_POWER_STATUS_EX {
  BYTE ACLineStatus;
  BYTE BatteryFlag;
  BYTE BatteryLifePercent;
  BYTE Reserved1;
  DWORD BatteryLifeTime;
  DWORD BatteryFullLifeTime;
  BYTE Reserved2;
  BYTE BackupBatteryFlag;
  BYTE BackupBatteryLifePercent;
  BYTE Reserved3;
  DWORD BackupBatteryLifeTime;
  DWORD BackupBatteryFullLifeTime;
} SYSTEM_POWER_STATUS_EX, *PSYSTEM_POWER_STATUS_EX, *LPSYSTEM_POWER_STATUS_EX;
#endif//_WIN32_WCE

typedef DWORD (*SETPROCPERMISSIONS)(DWORD newperms);
typedef HANDLE (*OPENTHREAD)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);
typedef BOOL (*WAITFORDEBUGEVENT)(LPDEBUG_EVENT lpDebugEvent, DWORD dwMilliseconds );
typedef BOOL (*CONTINUEDEBUGEVENT)(DWORD dwProcessId, DWORD dwThreadId, DWORD dwContinueStatus);
typedef ULONG (*GETTHREADCALLSTACK)(HANDLE hThrd, ULONG dwMaxFrames, LPVOID lpFrames[], DWORD dwFlags, DWORD dwSkip); 
typedef BOOL (*GETIDLETIMEEX)(DWORD dwProcessor,LPDWORD pdwIdleTime);
typedef BOOL (*GETSYSTEMPOWERSTATUSEX)(PSYSTEM_POWER_STATUS_EX pstatus, BOOL fUpdate);
typedef void (*GLOBALMEMORYSTATUS)(LPMEMORYSTATUS lpBuffer);
typedef BOOL (*GETPROCESSAFFINITY)(HANDLE handle, DWORD *affinity);
typedef BOOL (*GETTHREADAFFINITY)(HANDLE handle, DWORD *affinity);
typedef BOOL (*SETPROCESSAFFINITY)(HANDLE handle, DWORD affinity);
typedef BOOL (*SETTHREADAFFINITY)(HANDLE handle, DWORD affinity);

int set_thread_priority(int id, int priority);
void KillProcess(int id);
char_t *get_kdump_dir();
void set_kdump_dir(char_t *dir);
//char continue_debug(OPENTHREAD IOpenThread, GETTHREADCALLSTACK IGetThreadCallStack, 
//   WAITFORDEBUGEVENT IWaitForDebugEvent, CONTINUEDEBUGEVENT IContinueDebugEvent, CRASH_INFO **cinfo, int *debug_ps_count);
//char continue_debug(DEBUG_SESSION *dinfo);
void increment_num_processes_being_debugged();
void decrement_num_processes_being_debugged();
void init_lock();
void deinit_lock();
char debug_process(int rqstid, char_t *path, DWORD pid, OPENTHREAD IOpenThread, GETTHREADCALLSTACK IGetThreadCallStack, WAITFORDEBUGEVENT IWaitForDebugEvent, CONTINUEDEBUGEVENT IContinueDebugEvent, SOCKET client_socket, int cmd);
void debug_check();
bool DiscoveryServerInit(unsigned short port);
bool DiscoveryServerServe(DISCOVERY_PING *ping, DISCOVERY_PONG *pong);

#pragma warning(disable:4996)
//#pragma warning(default:4996)

#endif//CEMON