// PropPageOptions.cpp : implementation file
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
#include "devicelist.h"
#include "remotecpu.h"
#include "dlgRemotecpu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptions property page

IMPLEMENT_DYNCREATE(CPropPageOptions, CPropertyPage)

CPropPageOptions::CPropPageOptions() : PropPageBase(CPropPageOptions::IDD)
{
	//{{AFX_DATA_INIT(CPropPageOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPropPageOptions::~CPropPageOptions()
{
}

void CPropPageOptions::DoDataExchange(CDataExchange* pDX)
{
	PropPageBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageOptions)
	DDX_Control(pDX, IDC_EDIT_EMAILTO, m_edit_emailto);
	DDX_Control(pDX, IDC_EDIT_SMTP_SERVER, m_edit_smtp_server);
	DDX_Control(pDX, IDC_EDIT_SMTP_PORT, m_edit_smpt_port);
	//DDX_Control(pDX, IDC_EDIT_CEMON_PRIORITY, m_edit_cemon_priority);
   DDX_Control(pDX, IDC_EDIT_SNAP_INTERVAL, m_edit_snap_interval);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropPageOptions, PropPageBase)
	//{{AFX_MSG_MAP(CPropPageOptions)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_START, OnButtonCaptureStart)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_STOP, OnButtonCaptureStop)
   ON_BN_CLICKED(IDC_BUTTON_EMAIL_TEST, OnButtonEmailTest)
	ON_BN_CLICKED(IDC_CHECK_DONT_PROMPT_SHOW_REPORT, OnCheckDontPromptShowReport)
	ON_BN_CLICKED(IDC_CHECK_CAPTURE_ON_DEV, OnCheckCaptureOnDev)
   ON_BN_CLICKED(IDC_BUTTON_LOAD_CAPTURED_DATA, OnButtonLoadCapturedData)
   //ON_BN_CLICKED(IDC_BUTTON_SET_CEMON_PRIORITY, OnButtonSetCemonPriority)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptions message handlers

void CPropPageOptions::OnButtonCaptureStart() 
{
	// TODO: Add your control notification handler code here
	m_input_handler->OnButtonCaptureStart();
}

void CPropPageOptions::OnButtonCaptureStop() 
{
	// TODO: Add your control notification handler code here
	m_input_handler->OnButtonCaptureStop();
}

void CPropPageOptions::OnCheckCaptureOnDev() 
{
	// TODO: Add your control notification handler code here
   int start  = 0;
   CButton* chkbox = (CButton*)GetDlgItem(IDC_CHECK_CAPTURE_ON_DEV);
   if(chkbox)
   {
      int chk_state = chkbox->GetCheck();
      start = BST_CHECKED == chk_state ? 1:0;
      if(!m_input_handler->OnCheckCaptureOnDev(start))
      {
         chkbox->SetCheck(BST_CHECKED == chk_state ?  BST_UNCHECKED:BST_CHECKED);//revert to original state
      }
   }
}
void CPropPageOptions::OnCheckDontPromptShowReport() 
{
	// TODO: Add your control notification handler code here
   int prompt  = 0;
   CButton* chkbox = (CButton*)GetDlgItem(IDC_CHECK_DONT_PROMPT_SHOW_REPORT);
   if(chkbox)
   {
      int chk_state = chkbox->GetCheck();
      prompt = BST_CHECKED == chk_state ? 0:1;
      if(!m_input_handler->OnCheckDontPromptShowReport(prompt))
      {
         chkbox->SetCheck(BST_CHECKED == chk_state ?  BST_UNCHECKED:BST_CHECKED);//rever to original state
      }
   }
}

void CPropPageOptions::SetCheckDontPromptShowReport(bool chk_state) 
{
   CButton* chkbox = (CButton*)GetDlgItem(IDC_CHECK_DONT_PROMPT_SHOW_REPORT);
   if(chkbox)
   {
      chkbox->SetCheck(chk_state ?  BST_CHECKED:BST_UNCHECKED);
   }
}

void CPropPageOptions::OnButtonSetCemonPriority() 
{
	// TODO: Add your control notification handler code here
	m_input_handler->OnButtonSetCemonPriority();
}

void CPropPageOptions::OnButtonEmailTest()
{
   // TODO: Add your control notification handler code here
	UpdateData(TRUE);
   CWnd *wnd_server = GetDlgItem(IDC_EDIT_SMTP_SERVER);
   CWnd *wnd_port = GetDlgItem(IDC_EDIT_SMTP_PORT);
   CWnd *wnd_to = GetDlgItem(IDC_EDIT_EMAILTO);

   CString cs_server, cs_port, cs_to;
   wnd_server->GetWindowText(cs_server);
   STRING server((LPCTSTR)cs_server);
   server.trim(L" ");
   
   wnd_port->GetWindowText(cs_port);
   int port = atoiW((LPCTSTR)cs_port);

   wnd_to->GetWindowText(cs_to);
   STRING to((LPCTSTR)cs_to);
   to.trim(L" ");

   STRING from(L"stats@remotetaskmonitor.com");
   STRING subject(L"Remote Task Monitor - Test");
   STRING body(L"This is a test email from Remote Task Monitor.");

   EMAIL_DATA *edata = new EMAIL_DATA(m_input_handler, port, server, from, to, subject, body);
	if(edata)
	{
		CreateThread(0, 0, CDlgRemoteCPU::SendEmailThread, edata, 0, 0);
	}

   //m_input_handler->OnButtonSetCemonPriority();
}

void CPropPageOptions::SendEmail(const STRING &to, const STRING &subject, const STRING &body)
{
   // TODO: Add your control notification handler code here
	UpdateData(TRUE);
   CWnd *wnd_server = GetDlgItem(IDC_EDIT_SMTP_SERVER);
   CWnd *wnd_port = GetDlgItem(IDC_EDIT_SMTP_PORT);

   CString cs_server, cs_port;
   wnd_server->GetWindowText(cs_server);
   STRING server((LPCTSTR)cs_server);
   server.trim(L" ");
   
   wnd_port->GetWindowText(cs_port);
   int port = atoiW((LPCTSTR)cs_port);

   STRING from(L"stats@remotetaskmonitor.com");

   EMAIL_DATA *edata = new EMAIL_DATA(m_input_handler, port, server, from, to, subject, body);
	if(edata)
	{
		CreateThread(0, 0, CDlgRemoteCPU::SendEmailThread, edata, 0, 0);
	}

   //m_input_handler->OnButtonSetCemonPriority();
}


void CPropPageOptions::OnButtonLoadCapturedData() 
{
   // TODO: Add your control notification handler code here
   m_input_handler->OnButtonLoadCapturedData();
}

void CPropPageOptions::SetCheckCaptureOnDev(bool chk_state) 
{
   // TODO: Add your control notification handler code here
   int start  = 0;
   CButton* chkbox = (CButton*)GetDlgItem(IDC_CHECK_CAPTURE_ON_DEV);
   if(chkbox)
   {
      chkbox->SetCheck(chk_state ?  BST_CHECKED:BST_UNCHECKED);
   }
}

void CPropPageOptions::GetEmailTo(STRING &out)
{
	UpdateData(TRUE);
	CString txt;
	m_edit_emailto.GetWindowText(txt);
	out.set((LPCTSTR)txt);
	out.trim(L" ");
}

void CPropPageOptions::SetEmailTo(const char_t *s)
{
	m_edit_emailto.SetWindowText(s);
	UpdateData(FALSE);
}

void CPropPageOptions::GetSMTPServer(STRING &out)
{
	UpdateData(TRUE);
	CString txt;
	m_edit_smtp_server.GetWindowText(txt);
	out.set((LPCTSTR)txt);
	out.trim(L" ");
}

void CPropPageOptions::SetSMTPServer(const char_t *s)
{
	m_edit_smtp_server.SetWindowText(s);
	UpdateData(FALSE);
}

short CPropPageOptions::GetSMTPPort()
{
	UpdateData(TRUE);
	CString txt;
	m_edit_smpt_port.GetWindowText(txt);
	short port = (short)atoiW(txt);
   if(!port)
      port = 25;
   return port;
}

void CPropPageOptions::SetSMTPPort(const char_t* s)
{
	m_edit_smpt_port.SetWindowText(s);
	UpdateData(FALSE);
}
