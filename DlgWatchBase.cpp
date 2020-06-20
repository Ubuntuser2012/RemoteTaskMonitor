// DlgWatchBase.cpp : implementation file
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
#include "dlgWatchBase.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchBase dialog


CDlgWatchBase::CDlgWatchBase(int IDD, DEVICE * device, const STRING &email_to, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
	, m_device(device)
{
	//{{AFX_DATA_INIT(CDlgWatchCPUTotal)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	if(email_to)
		m_email_to.set(email_to);
}


/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPUTotal message handlers

BOOL CDlgWatchBase::OnInitDialog() 
{
	BOOL ret = CDialog::OnInitDialog();
	
	if(ret)
	{
	  if(m_email_to.size())
		GetDlgItem(IDC_EDIT_EMAIL_TO)->SetWindowText(m_email_to.s());
	}

	return ret;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//If emailto is set and server is set, then return true
BOOL CDlgWatchBase::EmailSetupOk()
{
   CString emailto;
   GetDlgItem(IDC_EDIT_EMAIL_TO)->GetWindowText(emailto);
   m_email_to.set((LPCTSTR)emailto);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   STRING server;
   mainWnd->m_options.GetSMTPServer(server);
   if(m_email_to.size()>0 && server.size()==0)
   {
      int ret1 = MessageBox(L"For the email notification, you must first setup an outgoing SMTP server in the Options tab.\n\nCancel Watch?", L"Email Setup", MB_YESNO);
      if(IDYES==ret1)
      {
         return false;
      }
   }
   CDialog::OnOK();
   return true;
}