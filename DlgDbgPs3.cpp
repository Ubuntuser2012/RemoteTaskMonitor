// DlgDbgPs3.cpp : implementation file
//

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "RemoteCPU.h"
#include "DlgDbgPs3.h"
#include "DlgWatchCfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPs3 dialog


CDlgDbgPs3::CDlgDbgPs3(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDbgPs3::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgDbgPs3)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgDbgPs3::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDbgPs3)
	DDX_Control(pDX, IDC_EDIT_PROCESS_PATH, m_edit_ps_path);
	DDX_Control(pDX, IDC_EDIT_EMAIL_TO, m_edit_email_to);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDbgPs3, CDialog)
	//{{AFX_MSG_MAP(CDlgDbgPs3)
	ON_BN_CLICKED(IDC_CHECK_SILENT_AFTER_CRASH, OnCheckSilentAfterCrash)
	ON_BN_CLICKED(IDC_CHECK_RESTART_AFTER_CRASH, OnCheckRestartAfterCrash)
	ON_BN_CLICKED(IDC_CHECK_PS_WATCH, OnCheckPsWatch)
	ON_BN_CLICKED(IDC_CHECK_RESET_DEVICE, OnCheckResetDevice)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPs3 message handlers

void CDlgDbgPs3::OnOk() 
{
}

void CDlgDbgPs3::OnCancel() 
{
	CDlgWatchCfg* parent = (CDlgWatchCfg*)GetParent();
	parent->OnCancel();
}

void CDlgDbgPs3::OnCheckPsWatch() 
{
	// TODO: Add your control notification handler code here
	
}

void CDlgDbgPs3::OnCheckResetDevice()
{
	// TODO: Add your control notification handler code here
 	CButton *btn = (CButton *)GetDlgItem(IDC_CHECK_RESET_DEVICE);
 	if(BST_CHECKED == btn->GetCheck())
	{
	  GetDlgItem(IDC_EDIT_PROCESS_PATH)->EnableWindow(FALSE);
	  GetDlgItem(IDC_CHECK_PS_WATCH)->EnableWindow(FALSE);
	}
	else
	{
	  GetDlgItem(IDC_EDIT_PROCESS_PATH)->EnableWindow(TRUE);
	  GetDlgItem(IDC_CHECK_PS_WATCH)->EnableWindow(TRUE);
	}
}

void CDlgDbgPs3::OnCheckSilentAfterCrash() 
{
	// TODO: Add your control notification handler code here
	
}

void CDlgDbgPs3::OnCheckRestartAfterCrash() 
{
	// TODO: Add your control notification handler code here
	
}
