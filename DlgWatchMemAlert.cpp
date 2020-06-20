// DlgWatchMemAlert.cpp : implementation file
//

#include "stdafx.h"


#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "resource.h"
#include "RemoteCPU.h"
#include "DlgWatchMemAlert.h"
#include "DlgRemoteCPU.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchMemAlert dialog


CDlgWatchMemAlert::CDlgWatchMemAlert(DEVICE * device, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWatchMemAlert::IDD, pParent)
	, m_device(device)
{
	//{{AFX_DATA_INIT(CDlgWatchMemAlert)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CDlgWatchMemAlert::~CDlgWatchMemAlert()
{
}

void CDlgWatchMemAlert::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWatchMemAlert)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWatchMemAlert, CDialog)
	//{{AFX_MSG_MAP(CDlgWatchMemAlert)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCELALL, OnCancelAll)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchMemAlert message handlers

void CDlgWatchMemAlert::OnDestroy()
{
   CDialog::OnDestroy();
}

void CDlgWatchMemAlert::OnOK() 
{
   CDialog::OnOK();
   DestroyWindow();
}

void CDlgWatchMemAlert::OnCancel() 
{
   CDialog::OnCancel();
   DestroyWindow();
}

BOOL CDlgWatchMemAlert::OnInitDialog() 
{
   BOOL ret = CDialog::OnInitDialog();


   return ret;
}

void CDlgWatchMemAlert::OnCancelAll() 
{
	AfxGetMainWnd()->SetTimer(CANCEL_MEM_WATCH, 100, 0);
	OnCancel();
}

void CDlgWatchMemAlert::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}
