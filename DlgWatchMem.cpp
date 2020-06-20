// DlgWatchMem.cpp : implementation file
//

#include "stdafx.h"
#include "../ds/ds.h"
extern "C" {
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
#include "listctrl2.h"
#include "dlgRemotecpu.h"
#include "dlgThreadPriority.h"
#include "dlgStartProcess.h"
#include "dlgWatchCfg.h"
#include "dlgWatchBase.h"
#include "dlgWatchMem.h"
#include "watchlist.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CDlgWatchMem dialog


CDlgWatchMem::CDlgWatchMem(DEVICE* device, const STRING &email_to, CWnd* pParent /*=NULL*/)
: CDlgWatchBase(CDlgWatchMem::IDD, device, email_to, pParent)
{
	//{{AFX_DATA_INIT(CDlgWatchMem)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgWatchMem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWatchMem)
	DDX_Control(pDX, IDC_EDIT_TIME_LIMIT, m_edit_time);
	DDX_Control(pDX, IDC_EDIT_MEMORY_LIMIT, m_edit_mem);
	DDX_Control(pDX, IDC_EDIT_EMAIL_TO, m_edit_emailto);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWatchMem, CDialog)
	//{{AFX_MSG_MAP(CDlgWatchMem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchMem message handlers

void CDlgWatchMem::OnOK() 
{
	CString emailto;
	CString tmp;

   if(EmailSetupOk())
   {
      GetDlgItem(IDC_EDIT_MEMORY_LIMIT)->GetWindowText(tmp);
	   int min_mem = _wtoi((LPCTSTR)tmp);

	   GetDlgItem(IDC_EDIT_TIME_LIMIT)->GetWindowText(tmp);
	   int duration = _wtoi((LPCTSTR)tmp);

	   CButton *btn = (CButton *)GetDlgItem(IDC_CHECK_RESET_DEVICE);
	   bool reset_device = (btn->GetCheck() == BST_CHECKED);

	   WATCH_ITEM *watch_item = m_device->getMemWatch();
	   if(!watch_item)
	   {
		   watch_item = WATCH_ITEM::create(min_mem, m_device, duration, m_email_to.s(), reset_device);
		   m_device->setMemWatch(watch_item);
	   }
	   else
	   {
		   watch_item->set(min_mem, m_device, duration, m_email_to.s(), reset_device);
	   }
      CDlgWatchBase::OnOK(); 
   }
}

void CDlgWatchMem::OnCancel() 
{
   //m_device->delMemWatch();

	CDialog::OnCancel();
}

BOOL CDlgWatchMem::OnInitDialog() 
{
   BOOL ret = CDlgWatchBase::OnInitDialog();
   if(ret)
   {
      WATCH_ITEM *watch_item = m_device->getMemWatch();
      if(watch_item)
      {
         STRING s;
         s.sprintf(L"%.02f", watch_item->m_min_mem);
         GetDlgItem(IDC_EDIT_MEMORY_LIMIT)->SetWindowText(s.s());
         s.sprintf(L"%i", watch_item->m_duration);
         GetDlgItem(IDC_EDIT_TIME_LIMIT)->SetWindowText(s.s());
      }
   }
   return ret;	
}
