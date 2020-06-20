#include "../common/types.h"
#include "../common/util.h"
#include <time.h>
#include "cemon.h"

#define MAX_EXEC_STR_LEN 128

#define MAX_DEBUGEES 64
#define DUMP_DIR_REG_PATH L"System\\ErrorReporting\\DumpSettings"
//\NAND Flash\APPS\DebugFiles\DumpFiles\Ce062712-01\Ce062712-01.kdmp

typedef struct _DEBUG_SESSION
{
	DWORD rqstid;
	HANDLE hthread;//the handle of the debugger thread
	DWORD tid;//debuger thread id
	DWORD pid;//debugee process id
	HANDLE done;
	HANDLE crash;
	HANDLE ack;
	OPENTHREAD IOpenThread;
	GETTHREADCALLSTACK IGetThreadCallStack;
	WAITFORDEBUGEVENT IWaitForDebugEvent;
	CONTINUEDEBUGEVENT IContinueDebugEvent;
	SOCKET so;
	int numPs;//the count of processes being debugged in this session, includes debugee and child processes
	int cmd;
	char_t path[MAX_RFILE_PATH];
	CRASH_INFO *cinfo;//a linked list of CRASH_INFO objects
	CRITICAL_SECTION lock;//lock for the two threads to access cinfo linked list
	DEBUG_PROCESS_ACK *dbg_ack;
}
DEBUG_SESSION;

typedef struct _DEBUGER_CONTEXT
{
	HANDLE done[MAX_DEBUGEES];
	HANDLE crash[MAX_DEBUGEES];
	HANDLE ack[MAX_DEBUGEES];
	DEBUG_SESSION *dinfo[MAX_DEBUGEES];
	int numevents;
}
DEBUGER_CONTEXT;

static char_t *dumpdir=0;
static DEBUGER_CONTEXT gdbgctx;

char continue_debug(DEBUG_SESSION *dinfo);
DWORD WINAPI debuger_proc(LPVOID lpParameter);

void set_kdump_dir(char_t *dir)
{
	int num_chars = wcslen(dir) + 1;
	if(dumpdir)
	{
		free(dumpdir);
	}
	dumpdir = (char_t*) malloc(num_chars * sizeof(dir[0]));
	if(dumpdir)
	{
		wcscpy(dumpdir, dir);
	}
}

char_t *get_kdump_dir()
{
	if(dumpdir)
	{
		return dumpdir;
	}
	else
	{
		DWORD err = 0;
		HKEY key;
		if(ERROR_SUCCESS != RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			DUMP_DIR_REG_PATH,
			0, 
			KEY_READ,
			&key))
		{
			err = GetLastError();
logg(L"RegOpenKeyEx \"%s\" err:%i\n", DUMP_DIR_REG_PATH, err);
		}
		else 
		{
			DWORD cb = 2048;
			dumpdir = (char_t *)malloc(cb);
			if(dumpdir)
			{
				DWORD type = REG_SZ;
				if(ERROR_SUCCESS != RegQueryValueEx(key, L"DumpDirectory", 0, &type, (LPBYTE)dumpdir, &cb))
				{
					err = GetLastError();
logg(L"RegQueryValueEx DumpDirectory err:%i\n", err);
				}
				//free(val);
			}
			RegCloseKey(key);
		}
	}

	return dumpdir;
}


void delete_cinfo(DEBUG_SESSION* dinfo)
{
	CRASH_INFO *cinfo = 0;
	if(dinfo->cinfo)
	{
		for(cinfo = dinfo->cinfo; cinfo; )
		{
			CRASH_INFO * tmp = cinfo;
			cinfo = cinfo->next;
			free(tmp);
		}
		dinfo->cinfo = 0;
	}
}
void delete_dinfo(DEBUG_SESSION* dinfo)
{
	DeleteCriticalSection(&dinfo->lock);
	free(dinfo);
}

void debug_ack(int i)//called by main thread
{
	DEBUG_SESSION *dinfo;

	if(i>=0 && i<MAX_DEBUGEES)
	{
		CloseHandle(gdbgctx.ack[i]);
		dinfo = gdbgctx.dinfo[i];

		//logg(L"dinfo %x, dinfo->ack %x\n", dinfo, dinfo->ack);

		if(dinfo && dinfo->dbg_ack)
		{
			//logg(L"socket_write_client %x, %i\n", dinfo->so, dinfo->dbg_ack->msg.sz);
			socket_write_clients((char*)dinfo->dbg_ack, dinfo->dbg_ack->msg.sz, CLIENT_SOCKET_STATE_CONNECTED, 0);
			free(dinfo->dbg_ack);
			dinfo->dbg_ack = 0;
		}
	}
}

void debug_crashed(int i)//called by main thread
{
	DEBUG_SESSION *dinfo;
	CRASH_INFO *cinfo;

	if(i>=0 && i<MAX_DEBUGEES)
	{
		dinfo = gdbgctx.dinfo[i];
		if(dinfo)
		{
			EnterCriticalSection(&dinfo->lock);
			if(dinfo->cinfo)
			{      
				for(cinfo = dinfo->cinfo; cinfo; cinfo = cinfo->next)
				{
					socket_write_clients((char*)cinfo, cinfo->msg.sz, CLIENT_SOCKET_STATE_CONNECTED, 0);
				}
				delete_cinfo(dinfo);
			}         
			LeaveCriticalSection(&dinfo->lock);
		}
	}
}

void debug_done(int i)//called by main thread
{
	DEBUG_SESSION *dinfo;

	if(i>=0 && i<MAX_DEBUGEES)
	{
		CloseHandle(gdbgctx.crash[i]);
		CloseHandle(gdbgctx.done[i]);

		dinfo = gdbgctx.dinfo[i];

		if(dinfo)
		{
			delete_cinfo(dinfo);
			delete_dinfo(dinfo);
		}

		if(gdbgctx.numevents-i-1>0)
		{
			memmove(&gdbgctx.done[i], &gdbgctx.done[i+1], (gdbgctx.numevents-i-1) * sizeof(HANDLE));
			memmove(&gdbgctx.ack[i], &gdbgctx.ack[i+1], (gdbgctx.numevents-i-1) * sizeof(HANDLE));
			memmove(&gdbgctx.crash[i], &gdbgctx.crash[i+1], (gdbgctx.numevents-i-1) * sizeof(HANDLE));
			memmove(&gdbgctx.dinfo[i], &gdbgctx.dinfo[i+1], (gdbgctx.numevents-i-1) * sizeof(DEBUG_SESSION *));
		}

		gdbgctx.numevents--;
	}
}

void debug_check()//called by main thread
{
	DWORD ret;
	//logg(L"debug_check gdbgctx.numevents %i\n", gdbgctx.numevents);
	//lock();
	ret = WaitForMultipleObjects(gdbgctx.numevents, gdbgctx.ack, FALSE, 0);
	//logg(L"debug_check gdbgctx.ack ret %i\n", ret-WAIT_OBJECT_0);
	if(ret>=WAIT_OBJECT_0 && ret<WAIT_OBJECT_0+(DWORD)gdbgctx.numevents)
	{
		int i = ret - WAIT_OBJECT_0;
		//logg(L"### debug_check gdbgctx.ack %i\n", i);
		debug_ack(i);
	}

	ret = WaitForMultipleObjects(gdbgctx.numevents, gdbgctx.crash, FALSE, 0);
	//logg(L"debug_check gdbgctx.crash ret %i\n", ret-WAIT_OBJECT_0);
	if(ret>=WAIT_OBJECT_0 && ret<WAIT_OBJECT_0+(DWORD)gdbgctx.numevents)
	{
		int i = ret - WAIT_OBJECT_0;
		//logg(L"### debug_check gdbgctx.crash %i\n", i);
		debug_crashed(i);
	}

	ret = WaitForMultipleObjects(gdbgctx.numevents, gdbgctx.done, FALSE, 0);
	//logg(L"debug_check gdbgctx.done ret %i\n", ret-WAIT_OBJECT_0);
	if(ret>=WAIT_OBJECT_0 && ret<WAIT_OBJECT_0+(DWORD)gdbgctx.numevents)
	{
		int i = ret - WAIT_OBJECT_0;
		//logg(L"### debug_check gdbgctx.done %i\n", i);
		debug_done(i);
	}

	//unlock();
}

char create_dbgr_handles(int i)
{
	char ret = 0;

	gdbgctx.done[i] = CreateEvent(0, 0, 0, 0);
	if(!gdbgctx.done[i])
	{
		logg(L"CreateEvent gdbgctx.done failed err: %i\n", GetLastError());
		goto Exit;
	}

	gdbgctx.ack[i] = CreateEvent(0, 0, 0, 0);
	if(!gdbgctx.ack[i])
	{
		logg(L"CreateEvent gdbgctx.ack failed err: %i\n", GetLastError());
		goto Exit;
	}

	gdbgctx.crash[i] = CreateEvent(0, 0, 0, 0);
	if(!gdbgctx.crash[i])
	{
		logg(L"CreateEvent gdbgctx.crash failed err: %i\n", GetLastError());
		goto Exit;
	}
	ret = 1;
Exit:
	return ret;
}
void close_dbgr_handles(int i)
{
	if(gdbgctx.done[i]) 
		CloseHandle(gdbgctx.done[i]);
	if(gdbgctx.ack[i]) 
		CloseHandle(gdbgctx.ack[i]);
	if(gdbgctx.crash[i]) 
		CloseHandle(gdbgctx.crash[i]);
}

char debug_process(
				   int rqstid, 
				   char_t *path, 
				   DWORD pid,
				   OPENTHREAD IOpenThread, 
				   GETTHREADCALLSTACK IGetThreadCallStack, 
				   WAITFORDEBUGEVENT IWaitForDebugEvent, 
				   CONTINUEDEBUGEVENT IContinueDebugEvent, 
				   SOCKET client_socket, 
				   int cmd)
{
	char ret = 0;
	DEBUG_SESSION *dinfo;

	//lock();
	if(gdbgctx.numevents >= MAX_DEBUGEES)
	{
		logg(L"Cannot debug any more processes\n");
		goto Exit;
	}
	if(!create_dbgr_handles(gdbgctx.numevents))
	{
		close_dbgr_handles(gdbgctx.numevents);
		goto Exit;
	}

	gdbgctx.dinfo[gdbgctx.numevents] = dinfo = (DEBUG_SESSION*) malloc(sizeof(DEBUG_SESSION));
	if(!dinfo)
	{
		logg(L"malloc failed on DEBUG_SESSION\n");
		close_dbgr_handles(gdbgctx.numevents);
		goto Exit;
	}
	memset(dinfo, 0, sizeof(DEBUG_SESSION));

	dinfo->rqstid = rqstid;
	dinfo->hthread = 0;
	dinfo->tid = 0;
	dinfo->pid = pid;
	dinfo->done = gdbgctx.done[gdbgctx.numevents];
	dinfo->ack = gdbgctx.ack[gdbgctx.numevents];
	dinfo->crash = gdbgctx.crash[gdbgctx.numevents];
	dinfo->IOpenThread = IOpenThread;
	dinfo->IGetThreadCallStack = IGetThreadCallStack;
	dinfo->IWaitForDebugEvent = IWaitForDebugEvent;
	dinfo->IContinueDebugEvent = IContinueDebugEvent;
	dinfo->so = client_socket;
	dinfo->numPs = 0;
	dinfo->cmd = cmd;
	if(path && wcslen(path) < sizeof(dinfo->path)/sizeof(dinfo->path[0]))
		wcscpy(dinfo->path, path);
	else 
		dinfo->path[0] = 0;
	dinfo->cinfo = 0;
	InitializeCriticalSection(&dinfo->lock);
	dinfo->dbg_ack = (DEBUG_PROCESS_ACK*) malloc(sizeof(DEBUG_PROCESS_ACK));

	if(!dinfo->dbg_ack)
	{
		logg(L"malloc failed on DEBUG_PROCESS_ACK\n");
		close_dbgr_handles(gdbgctx.numevents);
		delete_dinfo(dinfo);
		goto Exit;
	}

	memset(dinfo->dbg_ack, 0, sizeof(DEBUG_PROCESS_ACK));
	dinfo->dbg_ack->rqstid = rqstid;

	dinfo->hthread = CreateThread(0, 0, debuger_proc, dinfo, 0, &dinfo->tid);
	if(!dinfo->hthread)
	{
		logg(L"Failed to CreateThread debuger_proc %i\n", GetLastError());
		free(dinfo->dbg_ack);
		close_dbgr_handles(gdbgctx.numevents);
		delete_dinfo(dinfo);
		goto Exit;
	}

	CloseHandle(dinfo->hthread);
	gdbgctx.numevents++;
	ret = 1;

Exit:
	//unlock();
	return ret;
}



void process_frames(CallSnapshotEx *frame, unsigned long numFrames)
{
	unsigned long i;
	for(i=0; i<numFrames; i++)
	{
		logg(L"0x%x\t0x%x\t0x%x\t\n", frame[i].dwFramePtr, frame[i].dwReturnAddr, frame[i].dwCurProc);
	}

}

ULONG get_callstack(
					OPENTHREAD IOpenThread, 
					GETTHREADCALLSTACK IGetThreadCallStack, 
					DEBUG_EVENT *debug_event, 
					CallSnapshotEx *frames, 
					ULONG num_frames)
{
	ULONG num_frames_retrieved = 0;

	if(IOpenThread && IGetThreadCallStack && debug_event)
	{
		DWORD tid = debug_event->dwThreadId;
		HANDLE hthread = IOpenThread(THREAD_ALL_ACCESS, 0, tid);
		if(hthread)
		{
			num_frames_retrieved = IGetThreadCallStack(hthread, num_frames, (LPVOID)frames, STACKSNAP_EXTENDED_INFO, 0);
			if(num_frames_retrieved)
			{
				process_frames(frames, num_frames_retrieved);
			}
			CloseHandle(hthread);
		}
	}

	return num_frames_retrieved;
}

static ULONG handle_debug_exception(
									OPENTHREAD IOpenThread, 
									  GETTHREADCALLSTACK IGetThreadCallStack,
									  DEBUG_EVENT *debug_event, 
									  DWORD *dwContinueStatus,
									  CallSnapshotEx *frames,
									  ULONG *num_frames)

{
	ULONG num_frames_retrieved = 0;
	char_t exc[MAX_EXEC_STR_LEN];
	EXCEPTION_DEBUG_INFO* exception = &debug_event->u.Exception;

	if(EXCEPTION_BREAKPOINT != exception->ExceptionRecord.ExceptionCode)
	{
		*dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	}

	switch(exception->ExceptionRecord.ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_ACCESS_VIOLATION");
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
		break;
	case EXCEPTION_BREAKPOINT:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_BREAKPOINT");
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_DATATYPE_MISALIGNMENT");
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_FLT_DENORMAL_OPERAND");
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_FLT_DIVIDE_BY_ZERO");
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_FLT_INEXACT_RESULT");
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_FLT_INVALID_OPERATION");
		break;
	case EXCEPTION_FLT_OVERFLOW:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_FLT_OVERFLOW");
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_FLT_STACK_CHECK");
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_FLT_UNDERFLOW");
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_ILLEGAL_INSTRUCTION");
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_IN_PAGE_ERROR");
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_INT_DIVIDE_BY_ZERO");
		break;
	case EXCEPTION_INT_OVERFLOW:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_INT_OVERFLOW");
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_INVALID_DISPOSITION");
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_NONCONTINUABLE_EXCEPTION");
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_PRIV_INSTRUCTION");
		break;
	case EXCEPTION_SINGLE_STEP:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_SINGLE_STEP");
		break;
	case EXCEPTION_STACK_OVERFLOW:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"EXCEPTION_STACK_OVERFLOW");
		break;
	default:
		myswprintf(exc, MAX_EXEC_STR_LEN,  L"Unknown");
		break;
	}

	logg(L"### %i chance exception %s at address 0x%x, exception-code: 0x%08x, pid: 0x%x [%i]\n",
		exception->dwFirstChance,
		exc,
		exception->ExceptionRecord.ExceptionAddress,
		exception->ExceptionRecord.ExceptionCode, 
		debug_event->dwProcessId,
		debug_event->dwProcessId);

	if(exception->dwFirstChance == 1 && *dwContinueStatus == DBG_EXCEPTION_NOT_HANDLED)
	{
		num_frames_retrieved = get_callstack(IOpenThread, IGetThreadCallStack, debug_event, frames, *num_frames);
	}
	return num_frames_retrieved;
}

char continue_processing_debug_events(
									  OPENTHREAD IOpenThread, 
									  GETTHREADCALLSTACK IGetThreadCallStack,
									  DEBUG_EVENT *debug_event, 
									  DWORD *dwContinueStatus,
									  CallSnapshotEx *frames,
									  ULONG *num_frames,
									  DEBUG_SESSION *dinfo
									  )
{
	char ret = 1;
	ULONG num_frames_retrieved = 0;

	*dwContinueStatus = DBG_CONTINUE;

	switch(debug_event->dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
		{
			CREATE_PROCESS_DEBUG_INFO *info = &debug_event->u.CreateProcessInfo;

			CloseHandle(info->hThread);
			CloseHandle(info->hProcess);

			dinfo->numPs++;

			logg(L"Started process: 0x%x [%i], debugee count %i\n", info->hProcess, info->hProcess, dinfo->numPs);
			break;
		}
	case EXIT_PROCESS_DEBUG_EVENT:
		{
			dinfo->numPs--;
			ret = (dinfo->numPs == 0) ? 0:1;

			logg(L"Exited process with id: 0x%x [%i], debugee count %i\n", debug_event->dwProcessId, debug_event->dwProcessId, dinfo->numPs);
			break;
		}
	case EXIT_THREAD_DEBUG_EVENT:
		{
			logg(L"Exited thread with id: 0x%x [%i]\n", debug_event->dwThreadId, debug_event->dwThreadId);
			break;
		}
	case EXCEPTION_DEBUG_EVENT:
		{
			num_frames_retrieved = handle_debug_exception(
				IOpenThread, 
				IGetThreadCallStack,
				debug_event, 
				dwContinueStatus,
				frames,
				num_frames);
			ret = 0;
		}
	}
	*num_frames = num_frames_retrieved;
	return ret;
}

void handle_exception(DEBUG_SESSION *dinfo, DEBUG_EVENT* debug_event, CallSnapshotEx *frames, ULONG num_frames)
{
	CRASH_INFO* cinfo = 0;
	int sz = sizeof(CRASH_INFO) + sizeof(CallSnapshotEx) * num_frames;

	EnterCriticalSection(&dinfo->lock);
	if(!dinfo->cinfo)
	{//first
		dinfo->cinfo = (CRASH_INFO*) malloc(sz);
		if(dinfo->cinfo)
			cinfo = dinfo->cinfo;
		else
			logg(L"malloc failed on CRASH_INFO\n");
	}
	else
	{
		for(cinfo = dinfo->cinfo; cinfo->next; cinfo = cinfo->next);
		cinfo->next = (CRASH_INFO*) malloc(sz);
		if(cinfo->next)
			cinfo = cinfo->next;
		else
		{
			logg(L"malloc failed on CRASH_INFO.\n");
			cinfo = 0;
		}
	}
	if(cinfo)
	{
		ULONG i;
		memset(cinfo, 0, sz);
		for(i=0; i<num_frames; i++)
		{
			memcpy(&cinfo->frame[i], &frames[i], sizeof(CallSnapshotEx));
		}
		memcpy(&cinfo->debug_event, debug_event, sizeof(DEBUG_EVENT));
		cinfo->rqstid = dinfo->rqstid;
		cinfo->next = 0;
		cinfo->msg.cmd = COMM_CMD_CRASH_INFO;
		cinfo->msg.sz = sz;
		cinfo->msg.numelm = num_frames;

		if(!SetEvent(dinfo->crash))
			logg(L"SetEvent crash %x err %i\n", dinfo->crash, GetLastError());
	}
	LeaveCriticalSection(&dinfo->lock);

	KillProcess(debug_event->dwProcessId);
}

char continue_debug(DEBUG_SESSION *dinfo)/*
										 OPENTHREAD IOpenThread, 
										 GETTHREADCALLSTACK IGetThreadCallStack, 
										 WAITFORDEBUGEVENT IWaitForDebugEvent, 
										 CONTINUEDEBUGEVENT IContinueDebugEvent,
										 CRASH_INFO **cinfo,
										 int *debug_ps_count)*/
{
	char ret = 0;
	CallSnapshotEx frames[MAX_FRAMES];
	ULONG num_frames = MAX_FRAMES;

	memset(frames, 0, sizeof(frames));

#ifdef _WIN32_WCE
	if(dinfo->IWaitForDebugEvent && dinfo->IContinueDebugEvent)
#endif
	{
		DEBUG_EVENT debug_event = {0};
		DWORD dwContinueStatus = DBG_CONTINUE;

		//while(ret) // Wait till the process exits.
		{
			SetLastError( 0 ); 
#ifdef _WIN32_WCE
			if (!dinfo->IWaitForDebugEvent(&debug_event, INFINITE))
#else
			if (!WaitForDebugEvent(&debug_event, INFINITE))
#endif
			{
				DWORD err = GetLastError();
				if(WAIT_TIMEOUT == err || ERROR_SEM_TIMEOUT == err)
				{
					//logg(L"### WaitForDebugEvent timed out, err: 0x%x\n", err);
					ret = 1;
				}
				else
				{
					logg(L"WaitForDebugEvent FAILED err:%i - cannot debug!\n", err);
				}
			}
			else
			{
				ret = continue_processing_debug_events(
					dinfo->IOpenThread,
					dinfo->IGetThreadCallStack,
					&debug_event, 
					&dwContinueStatus,
					frames,
					&num_frames,
					dinfo);

				//logg(L"### num_frames %i, dwContinueStatus %u\n", num_frames, dwContinueStatus);

				if(dwContinueStatus == DBG_EXCEPTION_NOT_HANDLED &&
					debug_event.u.Exception.dwFirstChance == 1)
				{
					handle_exception(dinfo, &debug_event, frames, num_frames);
					dwContinueStatus = DBG_CONTINUE;
				}

				{
#ifdef _WIN32_WCE
					if(!dinfo->IContinueDebugEvent(
#else
					if(!ContinueDebugEvent(
#endif
						debug_event.dwProcessId,
						debug_event.dwThreadId,
						dwContinueStatus))
					{
						DWORD err = GetLastError();
						logg(L"ContinueDebugEvent FAILED err:%i - cannot debug!\n", err);
					}
				}
			}
		}
	}

	//logg(L"### continue_debug ret %i\n", ret);
	return ret;
}


DWORD WINAPI debuger_proc(LPVOID lpParameter)
{
	DWORD ret = 0;
	BOOL debug_started = 0;
	DEBUG_SESSION *dinfo = (DEBUG_SESSION *)lpParameter;
	STARTUPINFOW startupInfo;
	PROCESS_INFORMATION pinfo;

	memset(&startupInfo, 0, sizeof(STARTUPINFOW));
	memset(&pinfo, 0, sizeof(PROCESS_INFORMATION));
	startupInfo.cb = sizeof(STARTUPINFOW);

	if(dinfo->dbg_ack)
	{
		dinfo->dbg_ack->msg.cmd = dinfo->cmd;
		dinfo->dbg_ack->msg.sz = sizeof(DEBUG_PROCESS_ACK);
		dinfo->dbg_ack->pid = dinfo->pid;
	}

	if(dinfo->pid)
		debug_started = DebugActiveProcess(dinfo->pid);
	else if(dinfo->path)
		debug_started = CreateProcess(dinfo->path, 0, 0, 0, 0, DEBUG_PROCESS, 0, 0, &startupInfo, &pinfo);

	if(!debug_started)
	{
		if(dinfo->pid)
			logg(L"failed to attach to process with id %u, err: %i\n", dinfo->pid, GetLastError());
		else if(dinfo->path)
			logg(L"failed to start debugee \"%s\", err: %i\n", dinfo->path, GetLastError());

		if(dinfo->dbg_ack)
		{
			dinfo->dbg_ack->msg.set = 0;
			if(!SetEvent(dinfo->ack))
				logg(L"SetEvent ack %x err %i\n", dinfo->ack, GetLastError());
			//else
			//logg(L"### SetEvent ack %x\n", dinfo->ack);
		}
	}
	else
	{
		if(dinfo->pid) //debugging active process
		{
			logg(L"DebugActiveProcess with id %u\n", dinfo->pid);
		}
		else //debugging newly created process
		{
			//if we don't need these handles, close them
			CloseHandle(pinfo.hThread);
			CloseHandle(pinfo.hProcess);

			dinfo->pid = pinfo.dwProcessId;
			logg(L"CreateProcess \"%s\"\n", dinfo->path);
		}

		if(dinfo->dbg_ack)
		{
			dinfo->dbg_ack->msg.set = 1;
			if(!SetEvent(dinfo->ack))
				logg(L"SetEvent ack %x err %i\n", dinfo->ack, GetLastError());
			//else
			//logg(L"### SetEvent ack %x\n", dinfo->ack);
		}

		while(continue_debug(dinfo));
	}

	if(!SetEvent(dinfo->done))
		logg(L"SetEvent done %x err %i\n", dinfo->done, GetLastError());
	//else
	//logg(L"### SetEvent done %x\n", dinfo->done);

	//logg(L"### debuger_proc  done\n", dinfo->path);

	return ret;
}
