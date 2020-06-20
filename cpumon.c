//GetIdleTime https://msdn.microsoft.com/en-us/library/ms885626.aspx
//CeGetIdleTimeEx https://msdn.microsoft.com/en-us/library/gg154739.aspx
//CeGetProcessAffinity  https://msdn.microsoft.com/en-us/library/gg155315.aspx
//CeGetThreadAffinity  https://msdn.microsoft.com/en-us/library/gg154427.aspx
//CeGetTotalProcessors https://msdn.microsoft.com/en-us/library/gg156807.aspx
//CePowerOffProcessor https://msdn.microsoft.com/en-us/library/gg156304.aspx
//http://stackoverflow.com/questions/25041749/calculating-the-current-cpu-usage-on-a-windows-mobile-ce-device

#include "../common/types.h"
#include "../common/util.h"
#include <time.h>

#ifdef windows
#include "cemon.h"
#elif defined(linux)
#include "linuxrtm.h"
#endif

//extern  __declspec(dllimport) DWORD SetProcPermissions(DWORD newperms);
//extern  __declspec(dllimport) HANDLE OpenThread(DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwThreadId);

//Developing Windows CE Device Drivers: May a Regular Application Benefit from It?
//http://www.developer.com/ws/pc/article.php/3603731/Developing-Windows-CE-Device-Drivers-May-a-Regular-Application-Benefit-from-It.htm

//File System Filter Driver Tutorial
//http://www.codeproject.com/KB/system/fs-filter-driver-tutorial.aspx

//How to get individual process memory usage statistics in Windows Mobile
//http://www.codeproject.com/Tips/123149/How-to-get-individual-process-memory-usage-statist.aspx

// optimization for text shell to not snapshot heaps
#define TH32CS_SNAPNOHEAPS 0x40000000

#define DEFAULT_PRIORITY 100

#define IOCTL_KLIB_GETPROCMEMINFO 8 // get per-process id and r/w memory usage
#define MAX_INIT_NUM_PS 500
#define INC_NUM_PS 50
#define MAX_INIT_NUM_TS 1000
#define INC_NUM_TS 50

typedef struct _PROCVMINFO 
{
	HANDLE hProc; // process id
	DWORD cbRwMemUsed; // RW RAM used
}
PROCVMINFO, *PPROCVMINFO;

#ifdef windows
static SETPROCPERMISSIONS ISetProcPermissions;
static OPENTHREAD IOpenThread;
static WAITFORDEBUGEVENT IWaitForDebugEvent;
static CONTINUEDEBUGEVENT IContinueDebugEvent;
static GETTHREADCALLSTACK IGetThreadCallStack;
static GETIDLETIMEEX IGetIdleTimeEx;
static GETSYSTEMPOWERSTATUSEX IGetSystemPowerStatusEx;
static GLOBALMEMORYSTATUS IGlobalMemoryStatus;
static GETPROCESSAFFINITY IGetProcessAffinity;
static GETTHREADAFFINITY IGetThreadAffinity;
static SETPROCESSAFFINITY ISetProcessAffinity;
static SETTHREADAFFINITY ISetThreadAffinity;

#endif//windows

static char gdie = 0;
static char glogfile[MAX_FILE_PATH];
static SOCKET server_socket;
#ifdef windows
static const bool BLOCK = 0;
#elif defined(linux)
static const bool BLOCK = 1;
#endif
static char_t cemon_name[32] = L"cemon server";

void dieRtm()
{
	gdie=1;
	if(server_socket)
	{
		socket_server_end(server_socket, BLOCK);
		server_socket = 0;
	}
}

#ifdef _WIN32_WCE
#ifdef PKFUNCS_H
_inline bool CeGetProcVMInfo (int idxProc, DWORD cbSize, PROCVMINFO *pInfo)
{
	DWORD bytesReturned=0;
	return KernelLibIoControl((HANDLE) KMOD_CORE,
		IOCTL_KLIB_GETPROCMEMINFO, 
		NULL, 
		idxProc, 
		pInfo, cbSize, 
		&bytesReturned);
}
#endif//PKFUNCS_H
#endif//_WIN32_WCE

#if 1//def _X86_
DWORD __GetUserKData (DWORD dwOfst) { return 0;}
#endif

static 	void printVersion()
{
	char_t s[64] = {0};
	char_t version[8] = {0};
	_itow(RTMON_VERSION, version, 10);
	wcscpy(s, cemon_name);
	wcscat(s, version);
	wcscat(s, L"\n");
	logg(s);
}

static int readCemonFile(char_t* server_name, int num_chars, short *port)
{
#if defined(windows)
	FILE* fp;
	DWORD i = 0;
	char *name;
	char *line;
	char *path = 0;
	char_t *pathw = 0;

	if (!full_path(TXT("cemon.txt"), "cemon.txt", MAX_FILE_PATH, &pathw, &path))
	{
		logg(TXT("failed to get working directory name.\n"));
		goto Exit;
	}

	fp = fopen(path, "rt");
	if (!fp)
		logg(TXT("No \"%s\" file found. You can specify a name in this file that will be displayed on the RTM client.\n"), pathw ? pathw : TXT("?"));
	else
	{
		logg(TXT("processing %s\n"), pathw ? pathw : TXT("?"));
		if (fgetws(server_name, num_chars, fp))
		{
			int i;
			for (i = 0; i < num_chars; ++i)
			{
				if (server_name[i] == L':')
				{
					*port = _wtoi(&server_name[i + 1]);
					server_name[i] = 0;
					logg(TXT("using port %i\n"), *port);
					break;
				}
			}
		}
		fclose(fp);
	}
	logg(TXT("using server name %s\n"), server_name);
Exit:
	if (pathw)
		free(pathw);
	if (path)
		free(path);
	return 1;
#endif//windows
	return 0;
}
static void do_discovery( int reset, unsigned short server_port )
{
	DISCOVERY_PING ping;
	static DISCOVERY_PONG pong;

	if(reset)
	{
		DiscoveryServerInit(SERVER_DISCOVERY_PORT) ? 
		logg(TXT("discovery server initialized.\n")) : logg(TXT("discovery server failed to initialize.\n"));
	}
	memset(&pong, 0, sizeof(DISCOVERY_PONG));
	memset(&ping, 0, sizeof(DISCOVERY_PING));
	pong.instance= (__int64)&pong;
	pong.server_port = server_port;
	pong.connected_flag = GetNumClients() > 0 ? 1 : 0; 
	pong.version = RTMON_VERSION;
	wcscpy(pong.name, cemon_name);
	strcpy(pong.program, "rtmsrv");
	strcpy(ping.program, "rtmcli");
	ping.version = RTMON_VERSION;
	DiscoveryServerServe(&ping, &pong);
}

DWORD processesIndex(DWORD pid, EXECOBJ_DATA *pdata, DWORD pidsz)
{
	DWORD i = -1;
	for(i=0; i<pidsz; i++)
	{
		if(pdata[i].pid == pid)
			return i;
	}
	return i;
}

int saveSettings(DWORD priority, int capture_on_dev, DWORD interval_ms)
{
	int ret = 0;
	FILE* fp = 0;
	int sz = MAX_FILE_PATH;
	char *path=0;
	char_t *pathw=0;

    if(!full_path(TXT("rtm.cfg"), "rtm.cfg", sz, &pathw, &path))
	{
		logg(TXT("failed to get working directory name.\n"));
		goto Exit;
	}
	logg(TXT("saving %s\n"), pathw);
	fp=fopen(path, "wt");
	if(fp)
	{
		fprintf(fp, 
			"%lu:priority\n"
			"%i:capture_on_dev\n"
			"%lu:interval\n"
			, priority, capture_on_dev, interval_ms);
		fclose(fp);
		ret = 1;
	}
Exit:
	if(pathw)
		free(pathw);
	if(path)
		free(path);
	return ret;
}

int readSettings(COMM_INIT_SETTINGS* init_settings)
{
	int ret = 0;
	FILE* fp = 0;
	int sz = MAX_FILE_PATH;
	char *path=0;
	char_t *pathw=0;

	init_settings->priority = DEFAULT_PRIORITY;
	init_settings->snap_interval =  1000;
	init_settings->capture_on_dev = 0;
	init_settings->logging = 0;

    if(!full_path(TXT("rtm.cfg"), "rtm.cfg", sz, &pathw, &path))
	{
		logg(TXT("failed to get working directory name.\n"));
		goto Exit;
	}
	logg(TXT("loading %s\n"), pathw);
	fp=fopen(path, "rt");
	if(fp)
	{
		char *line = path;
		char field[128];
		int value=0;

		while(1)
		{
			if(feof(fp))
				break;
			if(!fgets(line, sz, fp))
				break;

			sscanf(line, "%i:%s", &value, field);

			if(strcmpA(field, "priority")==0)
			{ 
				init_settings->priority = value;
				logg(TXT("set priority %i\n"), init_settings->priority);
			}
			else if(strcmpA(field, "interval")==0)
			{ 
            if(value > 0 && value < MAX_SNAP_INTERVAL)
   				init_settings->snap_interval = value;
				logg(TXT("set snap interval %i\n"), init_settings->snap_interval);
			}
			else if(strcmpA(field, "logging")==0)
			{ 
				init_settings->logging = value;
				logg(TXT("log to file %s\n"), init_settings->logging ? TXT("enabled") : TXT("disabled"));
			}
			else if(strcmpA(field, "capture_on_dev")==0)
			{
				init_settings->capture_on_dev = value;
				logg(TXT("save to rtm file %s\n"), init_settings->capture_on_dev != 0 ? TXT("enabled"):TXT("disabled"));
				if(init_settings->capture_on_dev != 0)
				{
					int sz = MAX_FILE_PATH;
					char *path=0;
					char_t *pathw=0;
					char dt[MAX_FILE_PATH];
					char_t dtw[MAX_FILE_PATH], dtts[128];

					strtime(dtts, sizeof(dtts));
					sprintf(dt, ("log%s.rtm"), dtts);
#ifdef windows
					myswprintf(dtw, 
						(size_t)(sizeof(dtw)/sizeof(dtw[0])), 
						TXT("log%s.rtm"), dtts);
#endif						
					if(full_path(dtw, dt, sz, &pathw, &path))
					{
						strcpy(glogfile, path);
						logt("RTM file \"%s\" set to record data [%i]s\n", 
							glogfile, init_settings->capture_on_dev);
					}
					if(pathw)
						free(pathw);
					if(path)
						free(path);
				}
			}
		}
		fclose(fp);
		ret = 1;
	}
Exit:
	if(pathw)
		free(pathw);
	if(path)
		free(path);
	return ret;
}

int readThreadsFile(THREAD_NAME *tdata, DWORD tidsz)
{
#if defined(windows)
    FILE* fp;
	DWORD i=0;
	int sz = MAX_FILE_PATH;
	char *name;
	char *line;
	char *path=0;
	char_t *pathw=0;

    if(!full_path(TXT("threads.txt"), "threads.txt", sz, &pathw, &path))
	{
		logg(TXT("failed to get working directory name.\n"));
		goto Exit;
	}

	fp = fopen(path, "rt");
	if(!fp)
		logg(TXT("No \"%s\" file found. You can specify thread names in this file.\n"), pathw ? pathw : TXT("?"));
	else
	{
		logg(TXT("processing %s\n"), pathw ? pathw : TXT("?"));
		name = (char *)pathw;
		line = path;
		for(i=0; i<tidsz; i++)
		{
			if(feof(fp))
				break;
			if(!fgets(line, sz, fp))
				break;
			sscanf(path, "%lu:%s", &tdata[i].tid, name);
			if(strlen(name)>0 && strlen(name)<64)
			{
				wcscpy2(tdata[i].name, name, strlen(name));
				logg(TXT("%i:%s\n"), tdata[i].tid, tdata[i].name);
			}
			/*for(j=0; j<(int)strlen(name) && j<sz && j<sizeof(tdata[i].name)/sizeof(char_t); j++)
			{
			tdata[i].name[j] = name[j];
			}
			tdata[i].name[j] = 0;
			*/
			/*for(i=0; i<tidsz; i++)
			{
			if(tdata[i].tid == tid)
			{
			wcscpy2(tdata[i].name, tname, sizeof(tdata[i].name));
			break;
			}
			}
			*/
		}
		fclose(fp);
	}
Exit:
	if(pathw)
		free(pathw);
	if(path)
		free(path);
	return i;
#endif//windows
    return 0;
}

int alloc_parray(COMM_MESG **msg, DWORD *pidsz)
{
    DWORD sz=0;
    DWORD pidsz1 = *pidsz;
	*pidsz = pidsz1==0 ? MAX_INIT_NUM_PS : pidsz1 + INC_NUM_PS;
    sz =  sizeof(COMM_MESG) + (*pidsz) * sizeof(EXECOBJ_DATA);
    msg[COMM_CMD_PDATA] = reallocf(msg[COMM_CMD_PDATA], sz);
    if(!msg[COMM_CMD_PDATA])
	{
        logg(TXT("failed to allocate %i bytes of process memory. Exiting.\n"), sz);
		return 0;
    }
	return 1;
}

bool increasePsList(EXECOBJ_DATA **pdata, COMM_MESG **msg, DWORD *pidsz)
{
	bool ret = 0;
	DWORD sz = 0;
	DWORD originalSize = *pidsz;
	logg(TXT("increasing ps array size\n"));
	if(!alloc_parray(msg, pidsz))
	{
		logg(TXT("failed to allocate process memory. Exiting.\n"));
		goto Exit;
	}
	logg(TXT("%i -> %i\n"), originalSize, *pidsz);
	sz =  sizeof(COMM_MESG) + *pidsz * sizeof(EXECOBJ_DATA);
	//memset(msg[COMM_CMD_PDATA], 0, sz);
	msg[COMM_CMD_PDATA]->sz = sz;
	*pdata = (EXECOBJ_DATA*)(&msg[COMM_CMD_PDATA][1]);
	ret = 1;
Exit:
	return ret;
}


int alloc_tarray(COMM_MESG **msg, DWORD *tidsz)
{
	DWORD sz=0;
	DWORD tidsz1 = *tidsz;
	*tidsz = tidsz1==0 ? MAX_INIT_NUM_TS : tidsz1 + INC_NUM_TS;
	sz =  sizeof(COMM_MESG) + (*tidsz) * sizeof(EXECOBJ_DATA);
	msg[COMM_CMD_TDATA] = realloc(msg[COMM_CMD_TDATA], sz);
	if(!msg[COMM_CMD_TDATA])
	{
		logg(TXT("failed to allocate %i bytes of thread memory. Exiting.\n"), sz);
		return 0;
	}
	return 1;
}

bool increaseThList(EXECOBJ_DATA **tdata, COMM_MESG **msg, DWORD *tidsz)
{
	bool ret = 0;
	DWORD sz = 0;
	DWORD originalSize = *tidsz;
	logg(TXT("increasing th array size\n"));
	if(!alloc_tarray(msg, tidsz))
	{
		logg(TXT("failed to allocate thread memory. Exiting.\n"));
		goto Exit;
	}
	logg(TXT("%i -> %i\n"), originalSize, *tidsz);
	sz =  sizeof(COMM_MESG) + *tidsz * sizeof(EXECOBJ_DATA);
	//memset(msg[COMM_CMD_PDATA], 0, sz);
	msg[COMM_CMD_TDATA]->sz = sz;
	*tdata = (EXECOBJ_DATA*)(&msg[COMM_CMD_TDATA][1]);
	ret = 1;
Exit:
	return ret;
}

bool expandList(COMM_MESG **msg, DWORD *pidsz, DWORD *tidsz, DWORD n, bool isps)
{
	EXECOBJ_DATA *data = isps ? 
		(EXECOBJ_DATA*)(&msg[COMM_CMD_PDATA][1]) :
		(EXECOBJ_DATA*)(&msg[COMM_CMD_TDATA][1]);
	
	bool ret = 0;
	if(isps)
		ret = (n==*pidsz) ? increasePsList(&data, msg, pidsz) : 1;
	else
		ret = (n==*tidsz) ? increaseThList(&data, msg, tidsz) : 1;

	return ret;
}

#ifdef windows
int get_ps_path(int pid, char_t* path, int sz)
{
	int ret =0;
	char lRes2 = 0;

	HANDLE lProcSnapshot=CreateToolhelp32Snapshot(
		TH32CS_SNAPTHREAD|
		TH32CS_SNAPMODULE|
		TH32CS_SNAPNOHEAPS, pid);
	if(lProcSnapshot!=INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modentry;
		memset(&modentry, 0, sizeof(MODULEENTRY32));
		modentry.th32ProcessID=pid;
		modentry.dwSize = sizeof(MODULEENTRY32);
		lRes2=Module32First(lProcSnapshot, &modentry);
		while(lRes2)
		{
			if(pid == modentry.th32ProcessID)
			{
				ret = wcslen(modentry.szExePath)+1;
				if(sz > ret)
					wcscpy(path, modentry.szExePath);
				goto Exit;
			}
			lRes2 = Module32Next(lProcSnapshot,&modentry);
		}
Exit:
#ifdef _WIN32_WCE
		CloseToolhelp32Snapshot(lProcSnapshot);
#else
		CloseHandle(lProcSnapshot);
#endif
	}
	return ret;
}

void KillProcess(int id)
{
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE|PROCESS_CREATE_PROCESS,  FALSE, id);
	if(hProcess)
	{
		if(!TerminateProcess(hProcess, 0))
		{
			logg(TXT("TerminateProcess FAILED with error: %i\n"), GetLastError());
		}
		CloseHandle(hProcess);
	}
}

static BOOL GetProcessAffinityById(DWORD id, DWORD *affinity)
{
	BOOL ret = TRUE;
	if(IGetProcessAffinity)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,  FALSE, id);
		if(hProcess)
		{

	#ifdef _WIN32_WCE
			ret = IGetProcessAffinity(hProcess, affinity);
			if(!ret)
			{
				logg(TXT("CeGetProcessAffinity FAILED with error: %i\n"), GetLastError());
			}
	#else
			*affinity = rand() % 4;
	#endif
			CloseHandle(hProcess);
		}
	}
	return ret;
}

static BOOL GetThreadAffinity(HANDLE hThread, DWORD *affinity)
{
	BOOL ret = 0;
#ifdef _WIN32_WCE
	if(IGetThreadAffinity)
	{
		ret = IGetThreadAffinity(hThread, affinity);
	}
#else if windows
	*affinity = rand() % 4;
	ret = 1;
#endif
	return ret;
}

static BOOL SetProcessAffinity(DWORD id, DWORD affinity)
{
	BOOL ret = 0;
#ifdef _WIN32_WCE
	if(ISetProcessAffinity)
	{
		HANDLE hProcess = OpenProcess(
			PROCESS_TERMINATE|PROCESS_CREATE_PROCESS|PROCESS_SET_INFORMATION,  
			FALSE, id);
		if(hProcess)
		{	
			if(!ISetProcessAffinity(hProcess, affinity))
			{
				logg(TXT("CeSetProcessAffinity failed with err %i"), GetLastError());
			}
			else
			{
				ret = 1;
			}
			CloseHandle(hProcess);
		}
	}
#else if windows
	logg(TXT("SetProcessAffinity process-id %i, core %i"), id, affinity);
	ret = 1;
#endif
	return ret;
}

static BOOL SetThreadAffinity(DWORD id, DWORD affinity)
{
	BOOL ret = 0;
#ifdef _WIN32_WCE
	if(ISetThreadAffinity)
	{
		HANDLE hThread = IOpenThread ? IOpenThread( THREAD_ALL_ACCESS, FALSE, id ) : (HANDLE) id;
		if(hThread)
		{	
			if(!ISetThreadAffinity(hThread, affinity))
			{
				logg(TXT("CeSetThreadAffinity failed with err %i"), GetLastError());
			}
			else
			{
				ret = 1;
			}
			CloseHandle(hThread);
		}
	}
#else if windows
	logg(TXT("SetThreadAffinity thread-id %i, core %i"), id, affinity);
	ret = 1;
#endif
	return ret;
}

void debug_ps_ack(SOCKET client_socket, int cmd, DWORD pid, char ack)
{
	DEBUG_PROCESS_ACK msg;
	msg.msg.cmd = cmd;
	msg.msg.sz = sizeof(DEBUG_PROCESS_ACK);
	msg.msg.set = ack;
	msg.pid = pid;
	socket_write_client(client_socket, (char*)&msg, msg.msg.sz, BLOCK);
}
#endif//windows

int handle_process(int id, int cmd, SOCKET client_socket)
{
	int ret=0;
	char_t *path = 0;
	char_t *me = 0;
	int sz = MAX_RFILE_PATH;

	switch(cmd)
	{
	case COMM_CMD_KILL_PROCESS:
		KillProcess(id);
		break;
	case COMM_CMD_GET_PROCESS_PATH:
		{
			FILE_DATA *msg = 0;
			sz = sizeof(FILE_DATA) + (sz * sizeof(char_t));
			msg = (FILE_DATA *)malloc(sz);
			if(msg)
			{
				int pathsz;

				memset(msg, 0, sz);
				pathsz = get_ps_path(id, msg->data, sz);

				msg->msg.id = id;
				msg->msg.cmd = COMM_CMD_GET_PROCESS_PATH;
				msg->msg.sz = sz;

				socket_write_client(client_socket, (const unsigned char*)msg, msg->msg.sz, BLOCK);

				free(msg);
			}
		}
		break;
	case COMM_CMD_RESTART_PROCESS:
		path = (char_t *)malloc(sz * sizeof(char_t));
		if(path)
		{
			int pathsz = get_ps_path(id, path, sz);
			if(pathsz>0 && pathsz<=sz)
			{
#ifdef windows			
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE|PROCESS_CREATE_PROCESS,  FALSE, id);
				if(hProcess)
				{
					STARTUPINFOW startupInfo;
					PROCESS_INFORMATION pinfo;

					TerminateProcess(hProcess, 0);
					CloseHandle(hProcess);

					memset(&startupInfo, 0, sizeof(STARTUPINFOW));
					memset(&pinfo, 0, sizeof(PROCESS_INFORMATION));
					startupInfo.cb = sizeof(STARTUPINFOW);
					if(!CreateProcess(path, 0, 0, 0, 0, 0, 0, 0, &startupInfo, &pinfo))
					{
						logg(TXT("failed to restart \"%s\", err: %i\n"), path, GetLastError());
					}
				}
#elif defined(linux)
				KillProcess(id);
				start_ps(path);
#endif//windows
			}
			free(path);
		}
		break;
	case COMM_CMD_CLONE_PROCESS:
		path = (char_t *)malloc(sz * sizeof(char_t));
		if(path)
		{
			int pathsz = get_ps_path(id, path, sz);
			if(pathsz>0 && pathsz<=sz)
			{
#ifdef windows			
				STARTUPINFOW startupInfo;
				PROCESS_INFORMATION pinfo;

				memset(&startupInfo, 0, sizeof(STARTUPINFOW));
				memset(&pinfo, 0, sizeof(PROCESS_INFORMATION));
				startupInfo.cb = sizeof(STARTUPINFOW);
				if(!CreateProcess(path, 0, 0, 0, 0, 0, 0, 0, &startupInfo, &pinfo))
				{
					logg(TXT("failed to start clone \"%s\", err: %i\n"), path, GetLastError());
				}
#elif defined(linux)
				start_ps(path);
#endif//windows
			}
			free(path);
		}
		break;
	case COMM_CMD_START_PROCESS:
		path = (char_t*)id;
		if(path)
		{
#ifdef windows			
			STARTUPINFOW startupInfo;
			PROCESS_INFORMATION pinfo;

			memset(&startupInfo, 0, sizeof(STARTUPINFOW));
			memset(&pinfo, 0, sizeof(PROCESS_INFORMATION));
			startupInfo.cb = sizeof(STARTUPINFOW);
			if(!CreateProcess(path, 0, 0, 0, 0, 0, 0, 0, &startupInfo, &pinfo))
			{
				logg(TXT("failed to start process \"%s\", err: %i\n"), path, GetLastError());
			}
#elif defined(linux)
			start_ps(path);
#endif//windows
		}
		break;
	case COMM_CMD_SERVER_RESTART:
		me = (char_t*)malloc(sz*sizeof(char_t));
		if(me)
		{
#ifdef windows			
			if(GetModuleFileName(0, me, sz))
			{
				STARTUPINFOW startupInfo;
				PROCESS_INFORMATION pinfo;

				memset(&startupInfo, 0, sizeof(STARTUPINFOW));
				memset(&pinfo, 0, sizeof(PROCESS_INFORMATION));
				startupInfo.cb = sizeof(STARTUPINFOW);
				if(!CreateProcess(me, 0, 0, 0, 0, 0, 0, 0, &startupInfo, &pinfo))
				{
					logg(TXT("failed to restart cemon \"%s\", err: %i\n"), me, GetLastError());
				}
				else
				{
					gdie = 1;
				}
			}
#elif defined(linux)
			int pid = getpid();
			get_ps_path(pid, me, sz*sizeof(char_t));
			start_ps(path);
#endif//windows
			free(path);
		}
		break;
	}
	return ret;
}

int handle_thread(int id, int cmd)
{
	int ret=0;
	switch(cmd)
	{
	case COMM_CMD_KILL_THREAD:
#ifdef windows
		{			
#ifdef _WIN32_WCE
			HANDLE hThread = IOpenThread ? IOpenThread( THREAD_ALL_ACCESS, FALSE, id ) : (HANDLE) id;
#else
			HANDLE hThread = OpenThread( THREAD_TERMINATE, FALSE, id );
#endif
			if(hThread)
			{
				TerminateThread(hThread, 0);
				CloseHandle(hThread);
			}
		}
#elif defined(linux)
		kill_thread(id);
#endif//windows
		break;
	case COMM_CMD_SUSPEND_THREAD:
#ifdef windows	
		{		
#ifdef _WIN32_WCE
			HANDLE hThread = IOpenThread ? IOpenThread( THREAD_ALL_ACCESS, FALSE, id ) : (HANDLE) id;
#else
			HANDLE hThread = OpenThread( THREAD_TERMINATE, FALSE, id );
#endif
			if(hThread)
			{
				SuspendThread(hThread);
				CloseHandle(hThread);
			}
		}
#elif defined(linux)
		suspend_thread(id);
#endif//windows
		break;
	case COMM_CMD_RESUME_THREAD:
#ifdef windows	
		{		
#ifdef _WIN32_WCE
			HANDLE hThread = IOpenThread ? IOpenThread( THREAD_ALL_ACCESS, FALSE, id ) : (HANDLE) id;
#else
			HANDLE hThread = OpenThread( THREAD_TERMINATE, FALSE, id );
#endif
			if(hThread)
			{
				ResumeThread(hThread);
				CloseHandle(hThread);
			}
		}
#elif defined(linux)
		resume_thread(id);
#endif//windows
		break;
	}
	return ret;
}

#ifdef windows	
int set_thread_priority(int id, int priority)
{
	int ret = 0;
#ifdef _WIN32_WCE
	HANDLE hThread = IOpenThread ? IOpenThread( THREAD_ALL_ACCESS, FALSE, id ) : (HANDLE) id;
#else
	HANDLE hThread = OpenThread( THREAD_TERMINATE, FALSE, id );
#endif
	//logg(TXT("CeSetThreadPriority %i==#\n"), priority);
   	if(hThread)
	{
#ifdef _WIN32_WCE
		int ret = CeSetThreadPriority(hThread, priority);
#else
		int ret = SetThreadPriority(hThread, priority);
#endif
		if(!ret)
			logg(TXT("CeSetThreadPriority failed with err %i\n"), GetLastError());
		if(IOpenThread)
			CloseHandle(hThread);
		ret = 1;
	}
	return ret;
}

int get_thread_priority(int id)
{
	int ret = 0;
#ifdef _WIN32_WCE
	HANDLE hThread = IOpenThread ? IOpenThread( THREAD_ALL_ACCESS, FALSE, id ) : (HANDLE) id;
#else
	HANDLE hThread = OpenThread( THREAD_TERMINATE, FALSE, id );
#endif
	if(hThread)
	{
#ifdef _WIN32_WCE
		ret = CeGetThreadPriority(hThread);
#else
		ret = GetThreadPriority(hThread);
#endif
		if(IOpenThread)
			CloseHandle(hThread);
	}
	return ret;
}
#endif//windows

COMM_MESG *get_thread_names()
{
	COMM_MESG *cmsg = (COMM_MESG *)malloc(sizeof(COMM_MESG) + sizeof(THREAD_NAME) * MAX_THREAD_NAMES);
	if(cmsg)
	{
		THREAD_NAME *tname = 0;
		int tids = MAX_THREAD_NAMES;
		cmsg->cmd = COMM_CMD_TNAME;
		tname = (THREAD_NAME *)&cmsg[1];
		tids = readThreadsFile(tname, tids);
		cmsg->numelm = tids;
		cmsg->sz = sizeof(COMM_MESG) + sizeof(THREAD_NAME) * tids;
	}
	return cmsg;
}

int process_client_request(SOCKET so, unsigned char *buffer, int *head, int tail)
{
	COMM_MESG *msg = 0;
	msg = (COMM_MESG*)(&(buffer[*head]));
/*
logg(TXT("%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i\n"), 
   buffer[0], buffer[1], buffer[2], 
   buffer[3], buffer[4], buffer[5], buffer[6], 
   buffer[7], buffer[8], buffer[9], buffer[10], 
   buffer[11]);
logg(TXT("%p, head %i, tail %i\n"), msg, *head, tail);
*/
	while( (tail-(*head) > 0) && (tail-(*head) >= msg->sz) )
	{
//logg(TXT("tail-(*head)[%i] >= msg->sz [%i], msg->cmd %i\n"), tail-(*head), msg->sz, msg->cmd);
		//logg(TXT("tail-head = %i - %i\n"), tail, *head);
		switch(msg->cmd)
		{
		case COMM_CMD_NOOP:
			{
				*head += msg->sz;
				break;
			}
		case COMM_CMD_GET_PROCESS_PATH:
			handle_process(msg->id, COMM_CMD_GET_PROCESS_PATH, so);
			*head += msg->sz;
			break;
		case COMM_CMD_SERVER_RESTART:
			handle_process(msg->id, COMM_CMD_SERVER_RESTART, so);
			*head += msg->sz;
			break;
		case COMM_CMD_SET_PROCESS_AFFINITY:
			{
				SET_CORE *core_msg_p = (SET_CORE *)msg;
				SetProcessAffinity(msg->id, core_msg_p->affinity);
			}
			*head += msg->sz;
			break;
		case COMM_CMD_SET_THREAD_AFFINITY:
			{
				SET_CORE *core_msg_p = (SET_CORE *)msg;
				SetThreadAffinity(msg->id, core_msg_p->affinity);
			}
			*head += msg->sz;
			break;
		case COMM_CMD_GET_SNAPSHOT:
			{
				socket_set_client_state(so, CLIENT_SOCKET_STATE_SEND_SNAPINFO);
			}
			*head += msg->sz;
			break;

		case COMM_CMD_GET_MEMINFO:
			{
				//logg(TXT("Recvd command GetMemoryInfo: %s\n"), msg->show ? TXT("Yes") : TXT("No"));
				(msg->show) ?  
					socket_set_client_state(so, CLIENT_SOCKET_STATE_SEND_MEMINFO) :
					socket_unset_client_state(so, CLIENT_SOCKET_STATE_SEND_MEMINFO);
			}
			*head += msg->sz;
			break;

		case COMM_CMD_TNAME:
			{
				COMM_MESG *cmsg = get_thread_names();
				if(cmsg)
				{
					if(cmsg->numelm>0)
					{
						if(SOCKET_ERROR==socket_write(so, (const unsigned char*)cmsg, cmsg->sz, BLOCK))
						{
							socket_close_client(so);
						}
					}
					free(cmsg);
				}
			}
			*head += msg->sz;
			break;

		case COMM_CMD_KILL_PROCESS:
			handle_process(msg->id, COMM_CMD_KILL_PROCESS, so);
			*head += msg->sz;
			break;

		case COMM_CMD_CLONE_PROCESS:
			handle_process(msg->id, COMM_CMD_CLONE_PROCESS, so);
			*head += msg->sz;
			break;

		case COMM_CMD_START_PROCESS:
			{
				PROCESS_START *pmsg = (PROCESS_START*)msg;
				handle_process((int)pmsg->name, COMM_CMD_START_PROCESS, so);
			}
			*head += msg->sz;
			break;

		case COMM_CMD_RESTART_PROCESS:
			handle_process(msg->id, COMM_CMD_RESTART_PROCESS, so);
			*head += msg->sz;
			break;

		case COMM_CMD_KILL_THREAD:
			handle_thread(msg->id, COMM_CMD_KILL_THREAD);
			*head += msg->sz;
			break;

		case COMM_CMD_SUSPEND_THREAD:
			handle_thread(msg->id, COMM_CMD_SUSPEND_THREAD);
			*head += msg->sz;
			break;

		case COMM_CMD_RESUME_THREAD:
			handle_thread(msg->id, COMM_CMD_RESUME_THREAD);
			*head += msg->sz;
			break;

		case COMM_CMD_THREAD_PRIORITY:
			{
				THREAD_PRIORITY *tmsg = (THREAD_PRIORITY *)msg;
				set_thread_priority(msg->id, tmsg->priority);
			}
			*head += msg->sz;
			break;

		case COMM_CMD_CAPTURE_ON_DEV:
			{
				CAPTURE_ON_DEV *lmsg = (CAPTURE_ON_DEV *)msg;
				saveSettings(lmsg->priority, lmsg->capture_on_dev, lmsg->interval_ms);
				handle_process(0, COMM_CMD_SERVER_RESTART, so);
			}
			*head += msg->sz;
			break;

		case COMM_CMD_DEVICE_SHUTDOWN:
#ifdef _WIN32_WCE
			SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
#elif defined(windows)
			ExitWindowsEx(EWX_POWEROFF, EWX_FORCE);
#elif defined(linux)
			system("shutdown -h now");
#endif
			*head += msg->sz;
			break;

		case COMM_CMD_DEVICE_SUSPEND:
#ifdef _WIN32_WCE
			SetSystemPowerState(0, POWER_STATE_SUSPEND, POWER_FORCE);
#elif defined(windows)
			SetSystemPowerState(TRUE, TRUE);
#elif defined(linux)
			//Todo linux		
#endif
			*head += msg->sz;
			break;

		case COMM_CMD_DEVICE_RESTART:
#ifdef _WIN32_WCE
			SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
#elif defined(windows)
			ExitWindowsEx(EWX_REBOOT, EWX_FORCE);
#elif defined(linux)
			system("shutdown -r now");
#endif
			*head += msg->sz;
			break;

		case COMM_CMD_CLOSE:
			{
				socket_close_client(so);
				goto Exit;
			}
		case COMM_CMD_FILE:
			//TODO
			*head += msg->sz;
			break;
#ifdef windows
		case COMM_CMD_KDUMP_DIR:
			if(msg->get)
				get_kdump_dir();
			else
			{
				KDUMP_DIR *kmsg = (KDUMP_DIR *)msg;
				set_kdump_dir(kmsg->name);
			}
			*head += msg->sz;
			break;
		case COMM_CMD_DEBUG_NEW_PROCESS:
			{
				PROCESS_START *pmsg = (PROCESS_START*)msg;
				debug_process(pmsg->rqstid, pmsg->name, 0, IOpenThread, IGetThreadCallStack, IWaitForDebugEvent, IContinueDebugEvent, so, COMM_CMD_DEBUG_NEW_PROCESS);
			}
			*head += msg->sz;
			break;
		case COMM_CMD_DEBUG_ACTIVE_PROCESS:
			{
				PROCESS_START *pmsg = (PROCESS_START*)msg;
				debug_process(pmsg->rqstid, 0, pmsg->pid, IOpenThread, IGetThreadCallStack, IWaitForDebugEvent, IContinueDebugEvent, so, COMM_CMD_DEBUG_NEW_PROCESS);
			}
			*head += msg->sz;
			break;
#endif
		default:
			{
				goto Exit;
			}
		}
		msg = (COMM_MESG*)&buffer[*head];
	}//while( tail-(*head) < msg->sz )
Exit:
	return 1;
}


int service_clients(SOCKET server_socket, 
					const unsigned char* new_client_msg1, int new_client_msg1_sz, 
					const unsigned char* new_client_msg2, int new_client_msg2_sz)
{
	int ret = 0, sret = 0;
	int i;
	CLIENT *clients = 0;
	SOCKET client_socket = 0;

	if(!GetNumClients())
	{
		client_socket = socket_accept(server_socket, BLOCK);
		if(client_socket)
		{
			if(client_socket != INVALID_SOCKET)
			{
				socket_write_client(client_socket, new_client_msg1, new_client_msg1_sz, BLOCK);
				socket_write_client(client_socket, new_client_msg2, new_client_msg2_sz, BLOCK);
			}
		}
	}
	for(i=0; i < GetNumClients(); i++)
	{
		socket_read_clienti(i, process_client_request, BLOCK);
	}

#ifdef windows
	socket_remove_overdue_clients(1);//Todo linux, check if 1 param is ok to do
#endif
	ret = 1;
	return ret;
}

#ifdef windows
static void GetMemoryInfo(DWORD pid, EXECOBJ_DATA *pdata, DWORD p)
{
	HANDLE lProcSnapshot2 = 0;
#ifdef _WIN32_WCE
	HEAPLIST32 heapListEntry;
	HEAPENTRY32 heapEntry;
	lProcSnapshot2=CreateToolhelp32Snapshot(
		TH32CS_SNAPHEAPLIST|
		TH32CS_SNAPNOHEAPS,
		pid);
	if(lProcSnapshot2!=INVALID_HANDLE_VALUE)
	{
		bool lRes2 = 0;;
		memset(&heapListEntry,0,sizeof(HEAPLIST32));
		heapListEntry.dwSize=sizeof(HEAPLIST32);
		lRes2=Heap32ListFirst(lProcSnapshot2, &heapListEntry);
		//logg(TXT("pid:%i, err:%i\n"), procEntry.th32ProcessID, GetLastError());
		pdata[p].mem = 0;
		while(lRes2)
		{
			bool lRes3 = 0;
			memset(&heapEntry,0,sizeof(HEAPENTRY32));
			heapEntry.dwSize = sizeof(HEAPENTRY32);
			lRes3 = Heap32First(lProcSnapshot2,&heapEntry,heapListEntry.th32ProcessID,heapListEntry.th32HeapID);
			while(lRes3)
			{
				heapEntry.dwSize = sizeof(HEAPENTRY32);
				pdata[p].mem += heapEntry.dwBlockSize;
				lRes3 = Heap32Next(lProcSnapshot2,&heapEntry);
			}
			//if(!lRes2)
			//   logg(TXT("Heap32First err: %i\n"), GetLastError());

			lRes2=Heap32ListNext(lProcSnapshot2,&heapListEntry);
		}//while(lRes2)
		//if(!lRes2)
		//   logg(TXT("Heap32ListFirst err: %i\n"), GetLastError());

		if(lProcSnapshot2)
		{
			CloseToolhelp32Snapshot(lProcSnapshot2);
		}
		lProcSnapshot2 = 0;
	}//if(lProcSnapshot2!=INVALID_HANDLE_VALUE)
	//logg(TXT(" pid:%i heap: %iMB, err:%i\n"), procEntry.th32ProcessID, pdata[p].hsz/1024/1024, GetLastError());
#elif defined(windows)//#ifdef _windows_WCE
	{
		HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD |
			PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ,
			FALSE, pdata[p].pid );
		pdata[p].mem = 0;
		if(hProcess)
		{
			PROCESS_MEMORY_COUNTERS pmc;
			if(GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
			{
				pdata[p].mem = pmc.WorkingSetSize;
			}
			CloseHandle(hProcess);
		}
	}                   
#endif//#ifdef _windows_WCE
	if(lProcSnapshot2)
	{
#ifdef _WIN32_WCE
		CloseToolhelp32Snapshot(lProcSnapshot2);
#else
		CloseHandle(lProcSnapshot2);
#endif
		lProcSnapshot2 = 0;
	}
}

HANDLE createToolHelpSnapshot()
{
	HANDLE lProcSnapshot=CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS
		|TH32CS_SNAPTHREAD
		|TH32CS_SNAPMODULE
		|TH32CS_SNAPNOHEAPS
		,0);
	if(lProcSnapshot==INVALID_HANDLE_VALUE)
	{
		logg(TXT("Failed to create snapshot. Err: %i. Exiting.\n"), GetLastError());
		return 0;
	}

	return lProcSnapshot;
}

void deleteToolHelpSnapshot(HANDLE lProcSnapshot)
{
	if(lProcSnapshot)
	{
#ifdef _WIN32_WCE
		CloseToolhelp32Snapshot(lProcSnapshot);
#else
		CloseHandle(lProcSnapshot);
#endif
		lProcSnapshot = 0;
	}
}

bool getPsStats(HANDLE lProcSnapshot, DWORD *pidsz, COMM_MESG **msg, EXECOBJ_DATA *pdata, DWORD *_p)
{
	bool lRes = 0;
	DWORD p = 0;
	PROCESSENTRY32 procEntry;
	//logg(TXT("snapshot ok.\n");

	memset(&procEntry,0,sizeof(PROCESSENTRY32));
	procEntry.dwSize=sizeof(PROCESSENTRY32);
	lRes=Process32First(lProcSnapshot,&procEntry);
	if(!lRes)
	{
		logg(TXT("Process32First err %i.\n"), GetLastError());
	}

	for(p=0; lRes;)
	{
		if(p==*pidsz && !increasePsList(&pdata, msg, pidsz))
			goto Exit;

		pdata[p].pid = procEntry.th32ProcessID;
		memcpy(pdata[p].name, procEntry.szExeFile, sizeof(pdata[p].name));

		if(socket_is_any_client_state(CLIENT_SOCKET_STATE_SEND_MEMINFO))
		{
			GetMemoryInfo(procEntry.th32ProcessID, pdata, p);
		}
		GetProcessAffinityById(procEntry.th32ProcessID, &pdata[p].core);

		p++;
		lRes=Process32Next(lProcSnapshot,&procEntry);
		//if(!lRes)
		//	logg(TXT("Process32Next err %i.\n", GetLastError());
	}
	*_p = p;
	return 1;
Exit:
	return 0;
}

bool getThStats(HANDLE lProcSnapshot, DWORD *tidsz, COMM_MESG **msg, EXECOBJ_DATA *tdata, DWORD *_t)
{
	bool lRes = 0;
	DWORD t = 0;
	THREADENTRY32 threadEntry;
	FILETIME creatTime, exitTime, userTime, kernelTime;
	memset(&threadEntry,0,sizeof(THREADENTRY32));
	threadEntry.dwSize=sizeof(THREADENTRY32);
	lRes=Thread32First(lProcSnapshot,&threadEntry);
	if(!lRes)
		logg(TXT("Thread32First err %i.\n"), GetLastError());
	for(t=0; lRes; )
	{
		HANDLE hThread = 0;
		if(t==*tidsz)
		{
			DWORD sz = 0;
			if(!alloc_tarray(msg, tidsz))
			{
				logg(TXT("failed to allocate thread memory. Exiting.\n"));
				goto Exit;
			}
			sz =  sizeof(COMM_MESG) + (*tidsz) * sizeof(EXECOBJ_DATA);
			//memset(msg[COMM_CMD_TDATA], 0, sz);
			msg[COMM_CMD_TDATA]->sz = sz;
			tdata = (EXECOBJ_DATA*)(&msg[COMM_CMD_TDATA][1]);
		}
		tdata[t].tid = threadEntry.th32ThreadID;
		tdata[t].pid = threadEntry.th32OwnerProcessID;
#ifdef windows
#ifdef _WIN32_WCE
		if(!IOpenThread)
			hThread = (HANDLE) tdata[t].tid;
		else
			hThread = IOpenThread( THREAD_ALL_ACCESS, FALSE, tdata[t].tid );
#else
		//tdata[p].hThread = 0;
		hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tdata[t].tid );
#endif
#endif
		if(!hThread)
		{
#ifndef windows
			logg(TXT("OpenThread err %i.\n"), GetLastError());
#endif
		}
		else
		{
#ifdef _WIN32_WCE
			tdata[t].priority = CeGetThreadPriority(hThread);
#else
			tdata[t].priority = 0;//GetThreadPriority(hThread);
#endif
			//tdata[t].ft = now;
			if(!GetThreadAffinity(hThread, &tdata[t].core))
			{
				//logg(TXT(" failed to get thread affinity, err:%i\n"), GetLastError());
			}
			if(!GetThreadTimes(hThread, &creatTime, &exitTime, &kernelTime, &userTime))
			{
				logg(TXT("GetThreadTimes failed, err %i\n"), GetLastError());
				if(IOpenThread)
					CloseHandle(hThread);
				tdata[t].ft = 0;
				//tdata[t].ft.dwHighDateTime = 0;
				//tdata[t].ft.dwLowDateTime = 0;
			}
			else
			{
				//mytime(&tdata[t].ft);
	            tdata[t].ft = timeElapsed();

				//DWORD pi;
				tdata[t].cpuTimeSpent = 
               (unsigned long) addFileTime(&kernelTime, &userTime)/10000;
            //100-nanosecond value, convert to milliseconds
            //100 x 10^-9 = 10^-7 seconds
				//pi = processesIndex(tdata[t].pid, pdata, pidsz);
				//pdata[pi].cpuTimeSpent += tdata[t].cpuTimeSpent;
				CloseHandle(hThread);

				t++;
			}
		}
		lRes=Thread32Next(lProcSnapshot,&threadEntry);
		//if(!lRes)
		//	logg(TXT("Thread32Next err %i.\n", GetLastError());
	}
	*_t = t;
	return 1;
Exit:
	return 0;
}
#endif//windows

static __int64 GetFrequency()
{
	static LARGE_INTEGER frequency;
	if(frequency.QuadPart==0) 
	{
		QueryPerformanceFrequency(&frequency);
	}
	return frequency.QuadPart;
}

static void GetPowerStatus(MY_POWER_INFO *power_info)
{
	if(IGetSystemPowerStatusEx)
	{
		SYSTEM_POWER_STATUS_EX power;
		memset(&power, 0, sizeof(power));
		IGetSystemPowerStatusEx(&power, FALSE);
		switch(power.ACLineStatus)
		{
		case AC_LINE_ONLINE: 
			power_info->ac = 1; 
			break;
		case AC_LINE_UNKNOWN: 
		default:
			power_info->ac = 0; 
			break;
		}
		power_info->battery = power.BatteryFlag;
		power_info->battery_percent = power.BatteryLifePercent;
	}
	else
	{
		power_info->ac = 0;
		power_info->battery = 1;
		power_info->battery_percent = 51;
	}
}

static void GetMemoryStatus(DWORD *freeMem, DWORD *totalMem)
{
#ifdef _WIN32_WCE
	if(IGlobalMemoryStatus)
	{
		MEMORYSTATUS mem_status;
		memset(&mem_status, 0, sizeof(MEMORYSTATUS));
		mem_status.dwLength = sizeof(MEMORYSTATUS);
		IGlobalMemoryStatus(&mem_status);
		*freeMem = mem_status.dwAvailPhys/1024;
		*totalMem = mem_status.dwTotalPhys/1024;	
	}
#else if windows
	{
		MEMORYSTATUSEX mem_status_ex;
		memset(&mem_status_ex, 0, sizeof(MEMORYSTATUSEX));
		mem_status_ex.dwLength = sizeof(MEMORYSTATUSEX);
		if(GlobalMemoryStatusEx(&mem_status_ex))
		{
			*freeMem = (DWORD)mem_status_ex.ullAvailPhys/1024;
			*totalMem = (DWORD)mem_status_ex.ullTotalPhys/1024;
		}
	}
#endif
}

static BOOL GetCpuIdleTime(DWORD numProcessors, DWORD idle[1], LONGLONG frequency)
{
	BOOL ret = TRUE;
#ifdef _WIN32_WCE
	if(IGetIdleTimeEx)
	{
		DWORD i;
		for(i=1; i<=numProcessors && ret; ++i)
		{
			ret = IGetIdleTimeEx(i, &idle[i-1]);
			if(!ret)
			{
				logg(TXT("CeGetIdleTimeEx failed for core %i\n"), i);
			}
			//logg(TXT("idle[%i] %i\n"), i, idle[i]);
		}
	}
#else
	DWORD i;
	LONG   bufferLength = (numProcessors * 8) * sizeof(ULONG64);
	ULONG64 *processorIdleCycleTime = malloc(bufferLength);
	memset(processorIdleCycleTime, 0, bufferLength);
	ret = QueryIdleProcessorCycleTime(&bufferLength, processorIdleCycleTime);
	for(i=0; i<numProcessors; i++)
	{
		idle[i] = (DWORD)((frequency > 0) ? (processorIdleCycleTime[i] / frequency) : 0);
	}
	free(processorIdleCycleTime);
#endif//_WIN32_WCE

	return ret;
}

#ifdef POCKETPC2003_UI_MODEL//def _WIN32_WCE
int mainCRTStartup(){
	int argc=0;
	char** argv=0;
#elif (_WIN32_WCE>=0x800)
int WinMain(int argc, char_t *argv[]){
#elif	defined(_WIN32_WCE)
int wmain(int argc, char_t *argv[]){
#else
int main(int argc, char_t *argv[]){
#endif

	bool ret=0;
	DWORD sz=0;
	DWORD pidsz = 0;
	DWORD tidsz = 0;
	DWORD cbNeeded = 0;
	DWORD p = 0;
	DWORD t = 0;
	int j = 0;
	int numProcessors = 1;
	COMM_INIT_SETTINGS *init_settings;
	EXECOBJ_DATA *pdata = 0;
	EXECOBJ_DATA *tdata = 0;
	COMM_SNAPINFO *sdata = 0;
	long64 cpuTime = 0;
	long64 testRunTimeDelta = 0;
	HANDLE hProcess = 0;
	FILE* fp = 0;
	FILE* flog = 0;
	MY_SYSTEM_INFO *sysinfo;
#ifdef windows
	HMODULE coredll = 0;
#endif
	COMM_MESG *msg[COMM_CMD_TOTAL] = {0};
	short Port = SERVER_PORT;
	int send_to_clients = 0;
	int need_snapshot = 0;
	DWORD last_snapshot_at = 0;
	DWORD time_since_last_snapshot = 0;
	DWORD time_elapsed_since_start = 0;
	DWORD time_started_at = 0;
	bool reset_discovery = 0;

#ifdef windows
#ifdef _WIN32_WCE
	coredll = LoadLibrary(TXT("coredll.dll"));
	if(!coredll)
	{
		logg(TXT("LoadLibrary coredll.dll failed. Exiting.\n"));
		goto Exit;
	}
	logg(TXT("coredll loaded.\n"));
	IOpenThread = (OPENTHREAD) GetProcAddress(coredll, TXT("OpenThread"));
	if (!IOpenThread)
	{
		logg(TXT("OpenThread not found!\n"));
	}
	ISetProcPermissions = (SETPROCPERMISSIONS) GetProcAddress(coredll, TXT("SetProcPermissions"));
	if(!ISetProcPermissions)
	{
		logg(TXT("SetProcPermissions not found. Exiting.\n"));
		goto Exit;
	}
	else
	{
		logg(TXT("SetProcPermissions(0xFFFFFFFF).\n"));
		ISetProcPermissions(0xFFFFFFFF);
	}
	IWaitForDebugEvent = (WAITFORDEBUGEVENT) GetProcAddress(coredll, TXT("WaitForDebugEvent"));
	if(!IWaitForDebugEvent)
	{
		logg(TXT("WaitForDebugEvent not found!\n"));
	}
logg(TXT("WaitForDebugEvent found!\n"));
	IContinueDebugEvent = (CONTINUEDEBUGEVENT) GetProcAddress(coredll, TXT("ContinueDebugEvent"));
	if(!IContinueDebugEvent)
	{
		logg(TXT("ContinueDebugEvent not found!\n"));
	}
logg(TXT("ContinueDebugEvent found!\n"));
	IGetThreadCallStack = (GETTHREADCALLSTACK) GetProcAddress(coredll, TXT("GetThreadCallStack"));
	if(!IGetThreadCallStack)
	{
		logg(TXT("GetThreadCallStack not found!\n"));
	}
logg(TXT("GetThreadCallStack found!\n"));
	IGetIdleTimeEx = (GETIDLETIMEEX) GetProcAddress(coredll, TXT("CeGetIdleTimeEx"));
	if(!IGetIdleTimeEx)
	{
		logg(TXT("GetIdleTimeEx not found!\n"));
	}
logg(TXT("CeGetIdleTimeEx found!\n"));
	IGetSystemPowerStatusEx = (GETSYSTEMPOWERSTATUSEX) GetProcAddress(coredll, TXT("GetSystemPowerStatusEx"));
	if(!IGetSystemPowerStatusEx)
	{
		logg(TXT("GetSystemPowerStatusEx not found!\n"));
	}
logg(TXT("GetSystemPowerStatusEx found!\n"));
	IGlobalMemoryStatus = (GLOBALMEMORYSTATUS) GetProcAddress(coredll, TXT("GlobalMemoryStatus"));
	if(!IGlobalMemoryStatus)
	{
		logg(TXT("GlobalMemoryStatus not found!\n"));
	}
logg(TXT("GlobalMemoryStatus found!\n"));
	IGetProcessAffinity = (GETPROCESSAFFINITY) GetProcAddress(coredll, TXT("CeGetProcessAffinity"));
	if(!IGetProcessAffinity)
	{
		logg(TXT("CeGetProcessAffinity not found!\n"));
	}
logg(TXT("CeGetProcessAffinity found!\n"));
	IGetThreadAffinity = (GETTHREADAFFINITY) GetProcAddress(coredll, TXT("CeGetThreadAffinity"));
	if(!IGetThreadAffinity)
	{
		logg(TXT("CeGetThreadAffinity not found!\n"));
	}
logg(TXT("CeGetThreadAffinity found!\n"));
	ISetProcessAffinity = (SETPROCESSAFFINITY) GetProcAddress(coredll, TXT("CeSetProcessAffinity"));
	if(!ISetProcessAffinity)
	{
		logg(TXT("CeSetProcessAffinity not found!\n"));
	}
logg(TXT("CeSetProcessAffinity found!\n"));
	ISetThreadAffinity = (SETTHREADAFFINITY) GetProcAddress(coredll, TXT("CeSetThreadAffinity"));
	if(!ISetThreadAffinity)
	{
		logg(TXT("CeSetThreadAffinity not found!\n"));
	}
logg(TXT("CeSetThreadAffinity found!\n"));

#else//not _WIN32_WCE
	IOpenThread = (OPENTHREAD)OpenThread;
	IGetSystemPowerStatusEx = (GETSYSTEMPOWERSTATUSEX)NULL;
	//IWaitForDebugEvent = (WAITFORDEBUGEVENT)WaitForDebugEvent;
	//IContinueDebugEvent = (CONTINUEDEBUGEVENT)ContinueDebugEvent;
	//IGetThreadCallStack = 0;//(GETTHREADCALLSTACK)GetThreadCallStack; 
#endif
#endif//windows

logg(TXT("initializing random# generator\n"));
   srand(timeElapsed());
logg(TXT("initialized\n"));
#ifdef windows
//	get_kdump_dir();
#endif
#ifdef linux
	// We expect write failures to occur but we want to handle them where
	// the error occurs rather than in a SIGPIPE handler.
	signal(SIGPIPE, SIG_IGN);
	// Register our SIGTERM handler
	sigregister();
#endif

	printVersion();
	readCemonFile(cemon_name, sizeof(cemon_name) / sizeof(char_t), &Port);

#ifdef windows
    logg(TXT("Start ok. Cemon version %i.%02i\n"), RTMON_VERSION/100, RTMON_VERSION%100);
#elif defined(linux)
    logg(TXT("Start ok, linuxrtm version %i.%02i\n"), RTMON_VERSION/100, RTMON_VERSION%100);
#endif

	sz =  sizeof(COMM_MESG) + sizeof(COMM_INIT_SETTINGS);
	msg[COMM_CMD_INIT_SETTINGS] = malloc(sz);
	if(!msg[COMM_CMD_INIT_SETTINGS])
	{
		logg(TXT("failed to allocate %i bytes of init settings. Exiting.\n"), sz);
		goto Exit;
	}
	memset(msg[COMM_CMD_INIT_SETTINGS], 0, sz);
	msg[COMM_CMD_INIT_SETTINGS]->sz = sz;
	msg[COMM_CMD_INIT_SETTINGS]->cmd = COMM_CMD_INIT_SETTINGS;
	init_settings = (COMM_INIT_SETTINGS*)(&msg[COMM_CMD_INIT_SETTINGS][1]);
	readSettings(init_settings);

	//if(init_settings->logging)
		logg_begin();

	fp = 0;

#ifdef windows
    if(!winsock_init())
		goto Exit;
	logg(TXT("winsock_init ok.\n"));
#endif//windows

	set_thread_priority(GetCurrentThreadId(), init_settings->priority);
	server_socket = socket_server_init(Port, BLOCK);
	if(!server_socket)
		goto Exit;

	logg(TXT("socket_server_init ok.\n"));

	DiscoveryServerInit(SERVER_DISCOVERY_PORT) ? 
		logg(TXT("discovery server initialized.\n")) : logg(TXT("discovery server failed to initialize.\n"));

	sz =  sizeof(COMM_MESG) + sizeof(MY_SYSTEM_INFO);
	msg[COMM_CMD_SYSINFO] = malloc(sz);
	if(!msg[COMM_CMD_SYSINFO])
	{
		logg(TXT("failed to allocate %i bytes of sys info memory. Exiting.\n"), sz);
		goto Exit;
	}
	memset(msg[COMM_CMD_SYSINFO], 0, sz);
	msg[COMM_CMD_SYSINFO]->sz = sz;
	msg[COMM_CMD_SYSINFO]->cmd = COMM_CMD_SYSINFO;
	msg[COMM_CMD_SYSINFO]->id = RTMON_VERSION;
	sysinfo = (MY_SYSTEM_INFO*)(&msg[COMM_CMD_SYSINFO][1]);

#ifdef windows
	{
		sysinfo->os = MY_OS_WINCE;
		memset(&sysinfo->w, 0, sizeof(sysinfo->w));
		GetSystemInfo(&sysinfo->w);
		sysinfo->frequency = GetFrequency();
		//sysinfo->w.dwNumberOfProcessors = 4;//testing123
		numProcessors = sysinfo->w.dwNumberOfProcessors;
	}
#elif defined(linux)
	getSystemInfo(sysinfo);
	numProcessors = sysinfo->l.numberOfProcessors;
#endif
	if(numProcessors<1)
	{
		numProcessors = 1;
	}
	sz =  sizeof(COMM_MESG) + sizeof(COMM_SNAPINFO);
	msg[COMM_CMD_SNAPINFO] = malloc(sz);
	if(!msg[COMM_CMD_SNAPINFO])
	{
		logg(TXT("failed to allocate %i bytes of snap info memory. Exiting.\n"), sz);
		goto Exit;
	}
	memset(msg[COMM_CMD_SNAPINFO], 0, sz);
	msg[COMM_CMD_SNAPINFO]->cmd = COMM_CMD_SNAPINFO;
	msg[COMM_CMD_SNAPINFO]->numelm = 1;
	msg[COMM_CMD_SNAPINFO]->sz = sz;
	sdata = (COMM_SNAPINFO*)(&msg[COMM_CMD_SNAPINFO][1]);

	//logg(TXT("loop gdie %i.\n", gdie);

	time_started_at = timeElapsed();

	for(j=0; !gdie; j++) 
	{
		int num_clients = GetNumClients();

		time_elapsed_since_start = timeElapsed() - time_started_at;
		time_since_last_snapshot = timeElapsed() - last_snapshot_at;
		send_to_clients = (num_clients > 0 && socket_is_any_client_state(CLIENT_SOCKET_STATE_SEND_SNAPINFO)) ? 1: 0;
		need_snapshot = 
			(
			( 
			glogfile[0] && 
			( time_since_last_snapshot >= init_settings->snap_interval ) && 
			(init_settings->capture_on_dev < 0 || ((int)time_elapsed_since_start < init_settings->capture_on_dev * 1000)) 
			)
			||
			( send_to_clients && ( time_since_last_snapshot >= init_settings->snap_interval ) )
			);

//logg(TXT("send_to_clients %i, GetNumClients() %i, time_since_last_snapshot %u, last_snapshot_at %u, need_snapshot %i.\n"), 
//	send_to_clients, GetNumClients(), time_since_last_snapshot, last_snapshot_at, need_snapshot);

		if(need_snapshot)
		{
			HANDLE lProcSnapshot = 0;
			if(!msg[COMM_CMD_PDATA])
			{
				if(!alloc_parray(msg, &pidsz))
				{
					logg(TXT("failed to allocate %i bytes of process memory. Exiting.\n"), sz);
					goto Exit;
				}
			}
			sz =  sizeof(COMM_MESG) + pidsz * sizeof(EXECOBJ_DATA);
			memset(msg[COMM_CMD_PDATA], 0, sz);
			msg[COMM_CMD_PDATA]->sz = sz;
			pdata = (EXECOBJ_DATA*)(&msg[COMM_CMD_PDATA][1]);
			p = 0;

			if(!msg[COMM_CMD_TDATA])
			{
				if(!alloc_tarray(msg, &tidsz))
				{
					logg(TXT("failed to allocate %i bytes of thread memory. Exiting.\n"), sz);
					goto Exit;
				}
			}
			sz =  sizeof(COMM_MESG) + tidsz * sizeof(EXECOBJ_DATA);
			memset(msg[COMM_CMD_TDATA], 0, sz);
			msg[COMM_CMD_TDATA]->sz = sz;
			tdata = (EXECOBJ_DATA*)(&msg[COMM_CMD_TDATA][1]);
			t = 0;
#ifdef windows
			lProcSnapshot = createToolHelpSnapshot();
			if(!lProcSnapshot)
				goto Exit;
			if(!getPsStats(lProcSnapshot, &pidsz, msg, pdata, &p))
				goto Exit;
			if(!getThStats(lProcSnapshot, &tidsz, msg, tdata, &t))
				goto Exit;
#elif defined(linux)
			{
				getPsStats("/proc", 1, 0, msg, &pidsz, &tidsz, &p, &t);
			}
#endif
			mytime(&sdata->ft);

			sdata->now = timeElapsed();//milliseconds
            localTime(&sdata->dt);
			memset(&sdata->memstatus, 0, sizeof(MY_MEM_INFO));
#ifdef windows
			{
				GetMemoryStatus(&sdata->memstatus.freeMemKB , &sdata->memstatus.totalMemKB);
				GetCpuIdleTime(numProcessors, sdata->idle, sysinfo->frequency);
				GetPowerStatus(&sdata->power);
			}
#elif defined(linux)
			getMemoryInfo(&sdata->memstatus);
#endif

			if(glogfile[0])
			{
				flog = fopen(glogfile, "a+b");
				//logg(TXT("writing data\n");
			}

			if(j==0 && flog)
			{
				COMM_MESG *cmsg = 0;

				fwrite((char*)&msg[COMM_CMD_INIT_SETTINGS]->sz, sizeof(msg[COMM_CMD_INIT_SETTINGS]->sz), 1, flog);
				fwrite((char*)msg[COMM_CMD_INIT_SETTINGS], msg[COMM_CMD_INIT_SETTINGS]->sz, 1, flog);

				fwrite((char*)&msg[COMM_CMD_SYSINFO]->sz, sizeof(msg[COMM_CMD_SYSINFO]->sz), 1, flog);
				fwrite((char*)msg[COMM_CMD_SYSINFO], msg[COMM_CMD_SYSINFO]->sz, 1, flog);

				cmsg = get_thread_names();
				if(cmsg)
				{
					fwrite((char*)&cmsg->sz, sizeof(cmsg->sz), 1, flog);
					fwrite((char*)cmsg, cmsg->sz, 1, flog);
					free(cmsg);
				}
			}

//logg(TXT("send_to_clients %s\n"), send_to_clients ? TXT("yes") : TXT("no"));//TODO for testing
			if(send_to_clients)
			{
				socket_write_clients((char*)msg[COMM_CMD_SNAPINFO], msg[COMM_CMD_SNAPINFO]->sz, CLIENT_SOCKET_STATE_SEND_SNAPINFO, BLOCK);
			}
			if(flog)
			{
				fwrite((char*)&msg[COMM_CMD_SNAPINFO]->sz, sizeof(msg[COMM_CMD_SNAPINFO]->sz), 1, flog);
				fwrite((char*)msg[COMM_CMD_SNAPINFO], msg[COMM_CMD_SNAPINFO]->sz, 1, flog);
			}

			msg[COMM_CMD_PDATA]->cmd = COMM_CMD_PDATA;
			msg[COMM_CMD_PDATA]->numelm = p;
			msg[COMM_CMD_PDATA]->sz = sizeof(COMM_MESG) + p * sizeof(EXECOBJ_DATA);

			if(send_to_clients)
			{
				socket_write_clients((char*)msg[COMM_CMD_PDATA], msg[COMM_CMD_PDATA]->sz, CLIENT_SOCKET_STATE_SEND_SNAPINFO, BLOCK);
			}
			if(flog)
			{
				fwrite((char*)&msg[COMM_CMD_PDATA]->sz, sizeof(msg[COMM_CMD_PDATA]->sz), 1, flog);
				fwrite((char*)msg[COMM_CMD_PDATA], msg[COMM_CMD_PDATA]->sz, 1, flog);
			}

			msg[COMM_CMD_TDATA]->cmd = COMM_CMD_TDATA;
			msg[COMM_CMD_TDATA]->numelm = t;
			msg[COMM_CMD_TDATA]->sz = sizeof(COMM_MESG) + t * sizeof(EXECOBJ_DATA);
			if(send_to_clients)
			{
				socket_write_clients((char*)msg[COMM_CMD_TDATA], msg[COMM_CMD_TDATA]->sz, CLIENT_SOCKET_STATE_SEND_SNAPINFO, BLOCK);
			}
			if(flog)
			{
				fwrite((char*)&msg[COMM_CMD_TDATA]->sz, sizeof(msg[COMM_CMD_TDATA]->sz), 1, flog);
				fwrite((char*)msg[COMM_CMD_TDATA], msg[COMM_CMD_TDATA]->sz, 1, flog);
				fclose(flog);
				flog = 0;
			}

#ifdef windows
			deleteToolHelpSnapshot(lProcSnapshot);
#endif
			socket_unset_clients_state(CLIENT_SOCKET_STATE_SEND_SNAPINFO);

			last_snapshot_at = timeElapsed();

		}//if( need_snapshot )

		if(!glogfile[0])//Not capturing to device, so service clients
		{
            service_clients(server_socket,
                (char*)msg[COMM_CMD_INIT_SETTINGS], msg[COMM_CMD_INIT_SETTINGS]->sz,
                (char*)msg[COMM_CMD_SYSINFO], msg[COMM_CMD_SYSINFO]->sz);
		}
		 if(!BLOCK ||//using non blocking socket, so sleep
			 glogfile[0])//or capturing to device
		 {
			 DWORD default_snap_interval = atoiW(SNAP_INTERVAL);
			 (init_settings->snap_interval < default_snap_interval || init_settings->snap_interval > MAX_SNAP_INTERVAL) ?
				 Sleep(default_snap_interval) : Sleep(init_settings->snap_interval);
		 }
#ifdef windows
		debug_check();
#endif
		do_discovery( num_clients > GetNumClients(), Port );
	}//for(j=0; !gdie; j++) 


Exit:
	if(msg[COMM_CMD_PDATA])
	{
		free(msg[COMM_CMD_PDATA]);
		msg[COMM_CMD_PDATA]=0;
	}

	if(msg[COMM_CMD_TDATA])
	{
		free(msg[COMM_CMD_TDATA]);
		msg[COMM_CMD_TDATA]=0;
	}


	logg(TXT("Exited\n"));
	dieRtm();
	winsock_end();

#ifdef _WIN32_WCE
	if(coredll)
	{
		FreeLibrary(coredll);
	}
#endif
	if(fp)
	{
		fclose(fp);
	}

//	if(init_settings->logging)
		logg_end();

	return ret;
}
