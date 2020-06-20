// DlgWatchCPUTotal.cpp : implementation file
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
#include "dlgWatchCPUTotal.h"
#include "watchlist.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPUTotal dialog


CDlgWatchCPUTotal::CDlgWatchCPUTotal(DEVICE * device, const STRING &email_to, CWnd* pParent /*=NULL*/)
: CDlgWatchBase(CDlgWatchCPUTotal::IDD, device, email_to, pParent)
{
	//{{AFX_DATA_INIT(CDlgWatchCPUTotal)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgWatchCPUTotal::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWatchCPUTotal)
	DDX_Control(pDX, IDC_EDIT_TIME_LIMIT, m_edit_time);
	DDX_Control(pDX, IDC_EDIT_EMAIL_TO, m_edit_emailto);
	DDX_Control(pDX, IDC_EDIT_CPU_LIMIT, m_edit_cpu);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWatchCPUTotal, CDialog)
	//{{AFX_MSG_MAP(CDlgWatchCPUTotal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPUTotal message handlers

void CDlgWatchCPUTotal::OnOK() 
{
	CString emailto;
	CString tmp;
   CButton *btn;
   char_t *endptr = 0 ;

   if(EmailSetupOk())
   {
      GetDlgItem(IDC_EDIT_CPU_LIMIT)->GetWindowText(tmp);
	   float max_cpu = (float)wcstod((LPCTSTR)tmp, &endptr);

	   GetDlgItem(IDC_EDIT_TIME_LIMIT)->GetWindowText(tmp);
	   int duration = _wtoi((LPCTSTR)tmp);

	   btn = (CButton *)GetDlgItem(IDC_CHECK_RESET_DEVICE);
	   bool reset_device = (btn->GetCheck() == BST_CHECKED);

      CDlgWatchCPUTotal::StartCPUWatch(m_device, max_cpu, duration, m_email_to.s(), reset_device);

      CDlgWatchBase::OnOK(); 
   }
}

void CDlgWatchCPUTotal::StartCPUWatch(DEVICE *device, float max_cpu, int duration, const char_t *email_to, bool reset_device)
{
	   WATCH_ITEM *watch_item = device->getCPUWatch();
	   if(!watch_item)
	   {
		   watch_item = WATCH_ITEM::create(max_cpu, device, duration, email_to, reset_device);
		   device->setCPUWatch(watch_item);
	   }
	   else
	   {
		   watch_item->set(max_cpu, device, duration, email_to, reset_device);
	   }
}

BOOL CDlgWatchCPUTotal::OnInitDialog() 
{
	BOOL ret = CDlgWatchBase::OnInitDialog();
   if(ret)
   {
      WATCH_ITEM *watch_item = m_device->getCPUWatch();
      if(watch_item)
      {
         STRING s;
         s.sprintf(L"%.02f", watch_item->m_max_cpu);
         GetDlgItem(IDC_EDIT_CPU_LIMIT)->SetWindowText(s.s());
         s.sprintf(L"%i", watch_item->m_duration);
         GetDlgItem(IDC_EDIT_TIME_LIMIT)->SetWindowText(s.s());
      }
   }
   return ret;
}
