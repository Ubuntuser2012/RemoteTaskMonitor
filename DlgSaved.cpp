// DlgSaved.cpp : implementation file
//

#include "stdafx.h"
#include "../common/types.h"
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
#include "DlgRemoteCPU.h"
#include "DlgSaved.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSaved dialog


CDlgSaved::CDlgSaved(STRING &msg, STRING &fname, bool prompt_show_report, CWnd* pParent)
	: CDialog(CDlgSaved::IDD, pParent)
	, m_msg(msg.s())
	, m_fname(fname.s())
	, m_prompt_show_report(prompt_show_report)
{
	//{{AFX_DATA_INIT(CDlgSaved)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgSaved::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSaved)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSaved, CDialog)
	//{{AFX_MSG_MAP(CDlgSaved)
	ON_BN_CLICKED(IDC_CHECK_DO_NOT_PROMPT_AGAIN, OnCheckDoNotPromptAgain)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSaved message handlers

void CDlgSaved::OnOK() 
{	
	CDialog::OnOK();
	char_t *path1;
   if(m_fname.size())
   {
	   path1 = new char_t [(m_fname.size()+128) * sizeof(char_t)];
	   if(path1)
	   {
		   memcpy(path1, m_fname.s(), m_fname.size() * sizeof(char_t));
		   path1[m_fname.size()]=0;
		   CreateThread(0, 0, CDlgRemoteCPU::ShellExecuteThread, path1, 0, 0);
	   }
   }
}

BOOL CDlgSaved::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CWnd *wnd;
	CButton *btn;
	
	// TODO: Add extra initialization here
	wnd = GetDlgItem(IDC_STATIC_MESG);
	wnd->SetWindowText(m_msg.s());

	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();

	if (m_prompt_show_report)
	{
		btn = (CButton*)GetDlgItem(IDC_CHECK_DO_NOT_PROMPT_AGAIN);
		btn->ShowWindow(SW_SHOW);

		btn->SetCheck(!mainWnd->GetPromptShowReport());
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSaved::OnCheckDoNotPromptAgain()
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();

	// TODO: Add your control notification handler code here

	CButton *btn = (CButton*)GetDlgItem(IDC_CHECK_DO_NOT_PROMPT_AGAIN);

	mainWnd->SetPromptShowReport(btn->GetCheck() != BST_CHECKED);
	mainWnd->m_options.SetCheckDontPromptShowReport(!mainWnd->GetPromptShowReport());

}
