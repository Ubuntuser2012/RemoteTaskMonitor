// remotecpu.cpp : Defines the class behaviors for the application.
//

//If using UNICODE, set Link option entry point: wWinMainCRTStartup

#include "stdafx.h"
#include "../common/types.h"
#include "../common/util.h"
extern "C"{
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "devicelist.h"
#include "remotecpu.h"
#include "resource.h"
#include "dlgRemotecpu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRemoteCPUApp

BEGIN_MESSAGE_MAP(CRemoteCPUApp, CWinApp)
	//{{AFX_MSG_MAP(CRemoteCPUApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

STRING timeAsString();

/////////////////////////////////////////////////////////////////////////////
// CRemoteCPUApp construction

CRemoteCPUApp::CRemoteCPUApp()
{
	logg_begin();
}

CRemoteCPUApp::~CRemoteCPUApp()
{
	logg_end();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRemoteCPUApp object

CRemoteCPUApp theApp;
/////////////////////////////////////////////////////////////////////////////
// CRemoteCPUApp initialization

BOOL CRemoteCPUApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	logg(L"%s Start ==================================================\n", timeAsString().s());


	CDlgRemoteCPU dlg;
	if(m_lpCmdLine && *m_lpCmdLine)
	{
		DebugMode(m_lpCmdLine, dlg);
	}
	else
	{
#ifdef _AFXDLL
		Enable3dControls();			// Call this when using MFC in a shared DLL
#else
		Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

		m_pMainWnd = &dlg;
		int nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}

		// Since the dialog has been closed, return FALSE so that we exit the
		//  application, rather than start the application's message pump.
	}
	return FALSE;
}

int StartProcess(const char_t *path);

void CRemoteCPUApp::DebugMode(char_t *flag, CDlgRemoteCPU &mainWnd)
{
   STRING dir;
   mainWnd.GetMyCurrentDirectory(dir);

   if(strcmpW(flag, TXT("-debug"))==0)
   {
		dir.append(TXT("\\rtmonitor.exe"));
		logg(TXT("launching %s"), dir.s());
		StartProcess(dir.s());
	}
}