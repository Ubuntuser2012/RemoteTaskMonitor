// CDlgWatchCPUTotalAlert.cpp : implementation file
//

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
#include "DlgWatchCPUTotalAlert.h"
#include "DlgRemoteCPU.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgWatchCPUTotalAlert dialog


CDlgWatchCPUTotalAlert::CDlgWatchCPUTotalAlert(DEVICE *device, CWnd* pParent/* = NULL*/)
	: CDialog(CDlgWatchCPUTotalAlert::IDD, pParent)
	, m_device(device)
{
	//{{AFX_DATA_INIT(CDlgWatchCPUTotalAlert)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgWatchCPUTotalAlert::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWatchCPUTotalAlert)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWatchCPUTotalAlert, CDialog)
	//{{AFX_MSG_MAP(CDlgWatchCPUTotalAlert)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCELTHIS, OnCancelThisWatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPUTotalAlert message handlers

void CDlgWatchCPUTotalAlert::OnDestroy()
{
   CDialog::OnDestroy();
}

void CDlgWatchCPUTotalAlert::OnOK() 
{
   CDialog::OnOK();
   DestroyWindow();
}

void CDlgWatchCPUTotalAlert::OnCancel() 
{
   CDialog::OnCancel();
   DestroyWindow();

}

void CDlgWatchCPUTotalAlert::OnCancelThisWatch() 
{
   m_device->delTotalCPUWatch();
   DestroyWindow();
}
