// test3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../common/types.h"
#include "../common/util.h"
#include "minorversion.h"
#include "comm.h"
#include <stdio.h>
#include <windows.h>
#include <DbgHelp.h>

#include <time.h>
#include "../fileio/fileio.h"
#include "../ds/ds.h"

#pragma comment(lib, "DbgHelp.lib") 
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Advapi32.lib")

#pragma warning (disable : 4996 )//localtime_s

typedef BOOL  (WINAPI *MINIDUMP_WRITE_DUMP)(
											IN HANDLE hProcess,
											IN DWORD ProcessId,
											IN HANDLE hFile,
											IN MINIDUMP_TYPE DumpType,
											IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
											IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
											IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
											);

STRING timeAsString()
{
	char timestr[128];
	makeTimeString(timestr, sizeof(timestr));

	STRING ret;
	ret.set(timestr);
	return ret;
}

//This method can be used to have the current process be able to call MiniDumpWriteDump
BOOL SetDumpPrivileges()
{
	BOOL       fSuccess  = FALSE;
	HANDLE      TokenHandle = NULL;
	TOKEN_PRIVILEGES TokenPrivileges;
	
	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&TokenHandle))
	{
		printf("Could not get the process token");
		goto Cleanup;
	}
	
	TokenPrivileges.PrivilegeCount = 1;
	
	if (!LookupPrivilegeValue(NULL,
		SE_DEBUG_NAME,
		&TokenPrivileges.Privileges[0].Luid))
	{
		printf("Couldn't lookup SeDebugPrivilege name");
		goto Cleanup;
	}
	
	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	
	//Add privileges here.
	if (!AdjustTokenPrivileges(TokenHandle,
		FALSE,
		&TokenPrivileges,
		sizeof(TokenPrivileges),
		NULL,
		NULL))
	{
		printf("Could not revoke the debug privilege");
		goto Cleanup;
	}
	
	fSuccess = TRUE;
	
Cleanup:
	
	if (TokenHandle)
	{
		CloseHandle(TokenHandle);
	}
	
	return fSuccess;
}

bool ProcessDebugEvent(DEBUG_EVENT &debug_event, DWORD &dwContinueStatus)
{
	bool ret = true;
	STRING now, err, err2;
	char timestr[128];
	now.set(timeAsString().s());

	switch(debug_event.dwDebugEventCode)
	{
	case EXIT_PROCESS_DEBUG_EVENT:
		{
			err.sprintf(L"%s --- App quit", now.s());
			dwContinueStatus = DBG_CONTINUE;
			ret = false;
			break;
		}
	case EXCEPTION_DEBUG_EVENT:
		{
			dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
			EXCEPTION_DEBUG_INFO& exception = debug_event.u.Exception;
			switch( exception.ExceptionRecord.ExceptionCode)
			{
			case EXCEPTION_ACCESS_VIOLATION:
				err.sprintf(L"EXCEPTION_ACCESS_VIOLATION\n");
				break;
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
				err.sprintf(L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n");
				break;
			case EXCEPTION_BREAKPOINT:
				dwContinueStatus = DBG_CONTINUE;
				err.sprintf(L"EXCEPTION_BREAKPOINT\n");
				break;
			case EXCEPTION_DATATYPE_MISALIGNMENT:
				err.sprintf(L"EXCEPTION_DATATYPE_MISALIGNMENT\n");
				break;
			case EXCEPTION_FLT_DENORMAL_OPERAND:
				err.sprintf(L"EXCEPTION_FLT_DENORMAL_OPERAND\n");
				break;
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
				err.sprintf(L"EXCEPTION_FLT_DIVIDE_BY_ZERO\n");
				break;
			case EXCEPTION_FLT_INEXACT_RESULT:
				err.sprintf(L"EXCEPTION_FLT_INEXACT_RESULT\n");
				break;
			case EXCEPTION_FLT_INVALID_OPERATION:
				err.sprintf(L"EXCEPTION_FLT_INVALID_OPERATION\n");
				break;
			case EXCEPTION_FLT_OVERFLOW:
				err.sprintf(L"EXCEPTION_FLT_OVERFLOW\n");
				break;
			case EXCEPTION_FLT_STACK_CHECK:
				err.sprintf(L"EXCEPTION_FLT_STACK_CHECK\n");
				break;
			case EXCEPTION_FLT_UNDERFLOW:
				err.sprintf(L"EXCEPTION_FLT_UNDERFLOW\n");
				break;
			case EXCEPTION_ILLEGAL_INSTRUCTION:
				err.sprintf(L"EXCEPTION_ILLEGAL_INSTRUCTION\n");
				break;
			case EXCEPTION_IN_PAGE_ERROR:
				err.sprintf(L"EXCEPTION_IN_PAGE_ERROR\n");
				break;
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
				err.sprintf(L"EXCEPTION_INT_DIVIDE_BY_ZERO\n");
				break;
			case EXCEPTION_INT_OVERFLOW:
				err.sprintf(L"EXCEPTION_INT_OVERFLOW\n");
				break;
			case EXCEPTION_INVALID_DISPOSITION:
				err.sprintf(L"EXCEPTION_INVALID_DISPOSITION\n");
				break;
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				err.sprintf(L"EXCEPTION_NONCONTINUABLE_EXCEPTION\n");
				break;
			case EXCEPTION_PRIV_INSTRUCTION:
				err.sprintf(L"EXCEPTION_PRIV_INSTRUCTION\n");
				break;
			case EXCEPTION_SINGLE_STEP:
				err.sprintf(L"EXCEPTION_SINGLE_STEP\n");
				break;
			case EXCEPTION_STACK_OVERFLOW:
				err.sprintf(L"EXCEPTION_STACK_OVERFLOW\n");
				break;
			default:
				break;
			}
			
			if(exception.dwFirstChance == 1)
			{
				err2.sprintf(L"Exception-code: 0x%08x\nAt address 0x%x",
					//now.s(),
					exception.ExceptionRecord.ExceptionCode,
					exception.ExceptionRecord.ExceptionAddress);
			}

			err.append(err2);

			if(dwContinueStatus == DBG_EXCEPTION_NOT_HANDLED)
			{
				//MessageBox(0, err.s(), L"Remote Task Monitor experienced a problem", MB_ICONHAND);
				logg(err.s());
				ret = false;
			}

			
		}
	}
	return ret;
}

//int _tmain(int argc, _TCHAR* argv[])
int StartProcess(const wchar_t *path)
{   
	//   wchar_t path[] = L"C:\\oosman\\test\\test4\\debug\\test4.exe";
	DWORD dwContinueStatus = DBG_CONTINUE;
	STARTUPINFOW startupInfo;
	PROCESS_INFORMATION pinfo;
	memset(&startupInfo, 0, sizeof(STARTUPINFOW));
	memset(&pinfo, 0, sizeof(PROCESS_INFORMATION));
	startupInfo.cb = sizeof(STARTUPINFOW);
	
	//SetDumpPrivileges();

	if(!CreateProcess(path, //lpApplicationName
		0, //lpCommandLine
		0, //lpProcessAttributes
		0, //lpThreadAttributes
		0, //bInheritHandles
		PROCESS_QUERY_INFORMATION 
		| PROCESS_VM_READ 
		| DEBUG_ONLY_THIS_PROCESS 
		//| THREAD_ALL_ACCESS //dwCreationFlags
		, 0 //lpEnvironment
		, 0 //lpCurrentDirectory
		, &startupInfo
		, &pinfo))
	{
		DWORD err = GetLastError();
		logg(L"%s --- Failed to start \"%s\", error: 0x%x\n", timeAsString().s(), path, err);
	}
	else
	{
		bool ret = true;
		DEBUG_EVENT debug_event = {0};
		for(;ret;)
		{
			if (!WaitForDebugEvent(&debug_event, INFINITE))
				break;
			ret = ProcessDebugEvent(debug_event, dwContinueStatus);  // User-defined function, not API
			ContinueDebugEvent(debug_event.dwProcessId,
				debug_event.dwThreadId,
				dwContinueStatus);
		}
	}
	if(DBG_EXCEPTION_NOT_HANDLED == dwContinueStatus)
	{
		logg(L"%s --- Crash occurred\n", timeAsString().s());
		// open the file
		STRING dumpfile;
		dumpfile.sprintf(L"rtm_log-ver%.02f%c-%s.dmp", RTMON_VERSION/100.0f, RTMON_VERSION_MINOR, timeAsString().s());
		HANDLE hFile = CreateFile(dumpfile.s(),
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		
		MINIDUMP_EXCEPTION_INFORMATION *exceptionParam = NULL;
		MINIDUMP_USER_STREAM_INFORMATION *userStreamParam = NULL;
		MINIDUMP_CALLBACK_INFORMATION *callbackParam = NULL;
		
		//HMODULE hDbgHelp = LoadLibrary(L"DBGHELP.DLL");
		//MINIDUMP_WRITE_DUMP MiniDumpWriteDump_ = (MINIDUMP_WRITE_DUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
		
		if(!MiniDumpWriteDump(
			pinfo.hProcess,
			pinfo.dwProcessId,
			hFile,
			MiniDumpNormal,//MINIDUMP_TYPE
			exceptionParam,
			userStreamParam,
			callbackParam
			))
		{
			DWORD err = GetLastError();
			logg(L"%s --- MiniDumpWriteDump failed with error 0x%x\n", timeAsString().s(), err);
		}
      else
      {
         STRING msg;
         msg.sprintf(TXT("Remote Task Monitor has experienced a problem and log file was created: ")
            TXT("\"%s\"\n\n")
            TXT("Please email this file to remote_task_manager@yahoo.com so we can resolve this issue.\n"),
            dumpfile.s(),
            RTMON_VERSION/100.0f);
         MessageBox(0, msg.s(), L"Remote Task Monitor experienced a problem", MB_ICONEXCLAMATION);
		 logg(L"%s --- MiniDumpWriteDump succeeded '%s'\n", timeAsString().s(), dumpfile.s());
      }
		
		//CloseHandle(hDbgHelp);
		CloseHandle(hFile);
	}
	//crash();
	
	logg(L"%s End\n", timeAsString().s());

	return 0;
}

