// RemoteCPU.h : main header file for the RemoteCPU application
//

#if !defined(AFX_RemoteCPU_H__99AB7FC2_2AC1_4FF3_A237_7F2E377843C2__INCLUDED_)
#define AFX_RemoteCPU_H__99AB7FC2_2AC1_4FF3_A237_7F2E377843C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRemoteCPUApp:
// See grope.cpp for the implementation of this class
//
class CDlgRemoteCPU;

class CRemoteCPUApp : public CWinApp
{
public:
	CRemoteCPUApp();
	~CRemoteCPUApp();
	void DebugMode(char_t *path, CDlgRemoteCPU &mainWnd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemoteCPUApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRemoteCPUApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

enum {
	ACTION_NONE=0,
	ACTION_FIND,
	ACTION_REPLACE,
	ACTION_STOP
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RemoteCPU_H__99AB7FC2_2AC1_4FF3_A237_7F2E377843C2__INCLUDED_)
