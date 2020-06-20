// DlgWatchCfg.cpp : implementation file
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCfg dialog

//static's
char_t CDlgWatchCfg::m_ps_path[MAX_RFILE_PATH];
char_t CDlgWatchCfg::m_map_file_path[MAX_PATH*4];
char_t CDlgWatchCfg::m_email_to[MAX_PATH];

CDlgWatchCfg::CDlgWatchCfg(DWORD pid, wchar_t *pname, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWatchCfg::IDD, pParent)
   , m_input_handler((CDlgRemoteCPU *)pParent)
	, m_pid(pid)
	, m_pname(pname)
	, m_debug_new_ps(FALSE)
	, m_step(1)
{
//    memset(m_ps_path, 0, sizeof(m_ps_path));
//    memset(m_map_file_path, 0, sizeof(m_map_file_path));
//    memset(m_email_to, 0, sizeof(m_email_to));

   //{{AFX_DATA_INIT(CDlgWatchCfg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CDlgWatchCfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWatchCfg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWatchCfg, CDialog)
	//{{AFX_MSG_MAP(CDlgWatchCfg)
	ON_BN_CLICKED(IDNEXT, OnNext)
	ON_BN_CLICKED(IDPREVIOUS, OnPrevious)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCfg message handlers

void CDlgWatchCfg::OnOK() 
{
	char_t ps_path2[MAX_RFILE_PATH];
	bool watch_ps2 = false;
	bool reset_device = false;
	bool silent_mode = false;
	CButton * btn;

	DEVICE* device = m_input_handler->GetDevice(0);

	m_dlgdbg1.m_edit_ps_path.UpdateData(TRUE);
	m_dlgdbg1.m_edit_ps_path.GetWindowText(m_ps_path, sizeof(m_ps_path)/sizeof(char_t));

	m_dlgdbg2.m_edit_map_file_path.UpdateData(TRUE);
	m_dlgdbg2.m_edit_map_file_path.GetWindowText(m_map_file_path, sizeof(m_map_file_path)/sizeof(char_t));

	m_dlgdbg3.m_edit_email_to.UpdateData(TRUE);
	m_dlgdbg3.m_edit_email_to.GetWindowText(m_email_to, sizeof(m_email_to)/sizeof(char_t));

	if(!(m_ps_path && wcslen(m_ps_path)>0) && !m_pid)
		goto Exit;

	m_dlgdbg3.m_edit_ps_path.UpdateData(TRUE);
	m_dlgdbg3.m_edit_ps_path.GetWindowText(ps_path2, sizeof(ps_path2)/sizeof(char_t));

 	btn = (CButton *)m_dlgdbg3.GetDlgItem(IDC_CHECK_PS_WATCH);
 	watch_ps2 = (BST_CHECKED == btn->GetCheck());

 	btn = (CButton *)m_dlgdbg3.GetDlgItem(IDC_CHECK_RESET_DEVICE);
	reset_device = (BST_CHECKED == btn->GetCheck());

 	btn = (CButton *)m_dlgdbg3.GetDlgItem(IDC_CHECK_SILENT_AFTER_CRASH);
	silent_mode = (BST_CHECKED == btn->GetCheck());

   StartCrashWatch(device, m_debug_new_ps, m_pid, m_ps_path, m_map_file_path, m_email_to, 
                                   ps_path2, watch_ps2, reset_device, silent_mode);
Exit:
   CDialog::OnOK();
}

void CDlgWatchCfg::StartCrashWatch(DEVICE* device, bool debug_new_ps, DWORD pid, const char_t *ps_path, const char_t *map_file_path, const char_t *email_to, 
                                   const char_t *ps_path2, bool watch_ps2, bool reset_device, bool silent_mode)
{
	WATCH_ITEM* watch_item = WATCH_ITEM::create(debug_new_ps, pid, ps_path, map_file_path, email_to,
		ps_path2, watch_ps2, reset_device, silent_mode);
   if(watch_item && device)
   {
      device->debugProcess(watch_item);
   }
}

void CDlgWatchCfg::OnCancel() 
{
	CDialog::OnCancel();
}

void CDlgWatchCfg::OnNext() 
{
   // TODO: Add your control notification handler code here
   ShowPage(++m_step);	
}

void CDlgWatchCfg::OnPrevious() 
{
   // TODO: Add your control notification handler code here
   ShowPage(--m_step);	
}

void CDlgWatchCfg::ShowPage(int step)
{
   switch(step)
   {
   case 1:
      m_dlgdbg1.ShowWindow(SW_SHOW);
      m_dlgdbg2.ShowWindow(SW_HIDE);
      m_dlgdbg3.ShowWindow(SW_HIDE);
	  GetDlgItem(IDPREVIOUS)->EnableWindow(FALSE);
	  GetDlgItem(IDNEXT)->EnableWindow(TRUE);
      break;
   case 2:
      m_dlgdbg1.ShowWindow(SW_HIDE);
      m_dlgdbg2.ShowWindow(SW_SHOW);
      m_dlgdbg3.ShowWindow(SW_HIDE);
	  GetDlgItem(IDPREVIOUS)->EnableWindow(TRUE);
	  GetDlgItem(IDNEXT)->EnableWindow(TRUE);
      break;
   case 3:
      m_dlgdbg1.ShowWindow(SW_HIDE);
      m_dlgdbg2.ShowWindow(SW_HIDE);
      m_dlgdbg3.ShowWindow(SW_SHOW);
	  GetDlgItem(IDPREVIOUS)->EnableWindow(TRUE);
	  GetDlgItem(IDNEXT)->EnableWindow(FALSE);
      break;
   }
}

BOOL CDlgWatchCfg::OnInitDialog() 
{
	BOOL ret = CDialog::OnInitDialog();
   if(ret)
   {
      m_dlgdbg1.Create(IDD_DEBUG_PROCESS1, this);
      m_dlgdbg2.Create(IDD_DEBUG_PROCESS2, this);
      m_dlgdbg3.Create(IDD_DEBUG_PROCESS3, this);

      RECT r;
      m_dlgdbg1.GetWindowRect(&r);
      m_dlgdbg1.SetWindowPos(0, 0, 0, r.right-r.left, r.bottom-r.top, SWP_HIDEWINDOW);
      m_dlgdbg2.GetWindowRect(&r);
      m_dlgdbg2.SetWindowPos(0, 0, 0, r.right-r.left, r.bottom-r.top, SWP_HIDEWINDOW);
      m_dlgdbg3.GetWindowRect(&r);
      m_dlgdbg3.SetWindowPos(0, 0, 0, r.right-r.left, r.bottom-r.top, SWP_HIDEWINDOW);

      // TODO: Add extra initialization here

      if(m_ps_path)
      {
         m_dlgdbg1.m_edit_ps_path.SetWindowTextW(m_ps_path);
         m_dlgdbg3.m_edit_ps_path.SetWindowTextW(m_ps_path);
      }

      STRING s;
      if(m_pid && m_pname.s())
      {
         s.sprintf(L"[Process ID:%u] %s", m_pid, m_pname.s());
         m_dlgdbg1.m_edit_pid.SetWindowText(s.s());

         CButton * radio1 = (CButton *)m_dlgdbg1.GetDlgItem(IDC_RADIO_DEBUG_ACTIVE_PROCESS);
         radio1->SetCheck(BST_CHECKED);

      }
      else
      {
         CButton * radio1 = (CButton *)m_dlgdbg1.GetDlgItem(IDC_RADIO_DEBUG_ACTIVE_PROCESS);
         radio1->EnableWindow(FALSE);
         CButton * radio2 = (CButton *)m_dlgdbg1.GetDlgItem(IDC_RADIO_DEBUG_NEW_PROCESS);
         radio2->SetCheck(BST_CHECKED);
         CEdit * editpspath = (CEdit *)m_dlgdbg1.GetDlgItem(IDC_EDIT_PROCESS_PATH);
         editpspath->EnableWindow(TRUE);
      }

      if(m_map_file_path)
         m_dlgdbg2.m_edit_map_file_path.SetWindowTextW(m_map_file_path);

      if(m_email_to)
         m_dlgdbg3.m_edit_email_to.SetWindowTextW(m_email_to);

      ShowPage(m_step);
   }
   return ret;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgWatchCfg::OnRadioDebugActiveProcess() 
{
	// TODO: Add your control notification handler code here
	CStatic * staticwnd = (CStatic *)m_dlgdbg1.GetDlgItem(IDC_STATIC_PID);
	staticwnd->EnableWindow(TRUE);

	CEdit * editpspath = (CEdit *)m_dlgdbg1.GetDlgItem(IDC_EDIT_PROCESS_PATH);
	editpspath->EnableWindow(FALSE);

	m_debug_new_ps = FALSE;
}

void CDlgWatchCfg::OnRadioDebugNewProcess() 
{
	// TODO: Add your control notification handler code here
	CStatic * staticwnd = (CStatic *)m_dlgdbg1.GetDlgItem(IDC_STATIC_PID);
	staticwnd->EnableWindow(FALSE);

	CEdit * editpspath = (CEdit *)m_dlgdbg1.GetDlgItem(IDC_EDIT_PROCESS_PATH);
	editpspath->EnableWindow(TRUE);

	m_debug_new_ps = TRUE;
}
