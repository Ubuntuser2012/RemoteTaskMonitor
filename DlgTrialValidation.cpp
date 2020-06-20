// DlgTrialValidation.cpp : implementation file
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
#include "DlgRemoteCPU.h"
#include "DlgTrialValidation.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgTrialValidation dialog


CDlgTrialValidation::CDlgTrialValidation(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTrialValidation::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTrialValidation)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgTrialValidation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTrialValidation)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgTrialValidation, CDialog)
	//{{AFX_MSG_MAP(CDlgTrialValidation)
	ON_BN_CLICKED(IDC_CHECK_PROXY_USE, OnButtonUseProxy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTrialValidation message handlers

void CDlgTrialValidation::MakeSmaller()
{
   CWnd *wnd = GetDlgItem(IDC_STATIC_PROXY);
   RECT rect1;
   wnd->GetWindowRect(&rect1);

   GetWindowRect(&m_rect);
   MoveWindow(m_rect.left, m_rect.top, m_rect.right-m_rect.left,
      rect1.top-m_rect.top, TRUE);
}

void CDlgTrialValidation::MakeBigger()
{
   RECT rect1;
   GetWindowRect(&rect1);

   MoveWindow(rect1.left, rect1.top, m_rect.right-m_rect.left,
      m_rect.bottom-m_rect.top, TRUE);
}

void CDlgTrialValidation::OnButtonUseProxy() 
{
   CButton *btn = (CButton*) GetDlgItem(IDC_CHECK_PROXY_USE);
   if(btn->GetCheck() == BST_CHECKED)
      MakeBigger();
   else
      MakeSmaller();
}

BOOL CDlgTrialValidation::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
   if(m_hWnd)
	{
		GetDlgItem(IDC_EDIT_USER)->SetWindowText(m_user);
		GetDlgItem(IDC_EDIT_PASS)->SetWindowText(m_pass);
		GetDlgItem(IDC_EDIT_PORT)->SetWindowText(m_port);

      CButton *btn = (CButton*) GetDlgItem(IDC_CHECK_PROXY_USE);
      btn->SetCheck(m_use ? BST_CHECKED : BST_UNCHECKED);

      GetWindowRect(&m_rect);
      OnButtonUseProxy();

		CenterWindow(0);
      UpdateData(FALSE);
	}	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgTrialValidation::OnOK() 
{
	GetDlgItem(IDC_EDIT_PORT)->GetWindowText(m_port);
	GetDlgItem(IDC_EDIT_USER)->GetWindowText(m_user);
	GetDlgItem(IDC_EDIT_PASS)->GetWindowText(m_pass);

   CButton *btn = (CButton*) GetDlgItem(IDC_CHECK_PROXY_USE);
   m_use = BST_CHECKED == btn->GetCheck() ? true:false;
 
   CDialog::OnOK();
}

void CDlgTrialValidation::OnCancel()
{
   CDialog::OnCancel();
}
