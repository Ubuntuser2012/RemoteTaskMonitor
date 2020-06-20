// DlgError.cpp : implementation file
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
#include "DlgError.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgError dialog


CDlgError::CDlgError(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgError::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgError)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgError::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgError)
	DDX_Control(pDX, IDC_EDIT_URL1, m_edit_url1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgError, CDialog)
	//{{AFX_MSG_MAP(CDlgError)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgError message handlers


BOOL CDlgError::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgError::OnOK() 
{
	ShowWindow(SW_HIDE);
}

void CDlgError::OnCancel()
{
	SetWindowText(L"Error");
	m_edit_url1.SetWindowText(L"");
	m_edit_url1.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_MESG1)->SetWindowText(L"");
	ShowWindow(SW_HIDE);
}

void CDlgError::SetMessage(STRING &title, STRING &msg, STRING &url) 
{
	if(m_hWnd)
	{
		if(title.size())
			SetWindowText(title.s());
		if(msg.size())
			GetDlgItem(IDC_STATIC_MESG1)->SetWindowText(msg.s());
		if(url.size())
			m_edit_url1.SetWindowText(url.s());
		else
			m_edit_url1.ShowWindow(SW_HIDE);

		ShowWindow(SW_SHOW);
		CenterWindow(0);
	}
}
