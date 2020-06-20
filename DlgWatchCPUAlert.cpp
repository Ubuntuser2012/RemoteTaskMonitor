// DlgWatchCPUAlert.cpp : implementation file
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
#include "DlgWatchCPUAlert.h"
#include "DlgRemoteCPU.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPUAlert dialog


CDlgWatchCPUAlert::CDlgWatchCPUAlert(DEVICE * device, WATCH_ITEM *watch_item, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWatchCPUAlert::IDD, pParent)
	, m_device(device)
	, m_watch_item(watch_item)
{
	//{{AFX_DATA_INIT(CDlgWatchCPUAlert)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CDlgWatchCPUAlert::~CDlgWatchCPUAlert()
{
}

void CDlgWatchCPUAlert::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWatchCPUAlert)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWatchCPUAlert, CDialog)
	//{{AFX_MSG_MAP(CDlgWatchCPUAlert)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCELALL, OnCancelAll)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDCANCELTHIS, OnCancelThisWatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPUAlert message handlers

void CDlgWatchCPUAlert::OnDestroy()
{
   CDialog::OnDestroy();
}

void CDlgWatchCPUAlert::OnOK() 
{
   CDialog::OnOK();
   DestroyWindow();
}

void CDlgWatchCPUAlert::OnCancel() 
{
   CDialog::OnCancel();
   DestroyWindow();

}

BOOL CDlgWatchCPUAlert::OnInitDialog() 
{
   BOOL ret = CDialog::OnInitDialog();


   return ret;
}

void CDlgWatchCPUAlert::OnCancelAll() 
{
	AfxGetMainWnd()->SetTimer(CANCEL_CPU_WATCH, 100, 0);
	OnCancel();
}

void CDlgWatchCPUAlert::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}

void CDlgWatchCPUAlert::OnCancelThisWatch() 
{
    if(m_watch_item)
    {
		EXECOBJ *execobj = 0;
		if(m_watch_item->m_pid && m_watch_item->m_tid)
		{
			PROCESS *p = m_device->getProcessById(m_watch_item->m_pid);
			if(p)
				execobj = p->getThreadById(m_watch_item->m_tid);
		}
		else if(m_watch_item->m_pid)
		{
			execobj = m_device->getProcessById(m_watch_item->m_pid);
		}
		if(execobj)
		{
			execobj->setCPUWatch(0);
		}
       delete m_watch_item;
    }
}
