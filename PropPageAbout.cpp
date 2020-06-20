// PropPageAbout.cpp : implementation file
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
#include "gdgraph.h"
#include "history.h"
#include "watchlist.h"
#include "device.h"
#include "devicelist.h"
#include "remotecpu.h"
#include "dlgRemotecpu.h"
#include "PropPageAbout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageAbout property page

IMPLEMENT_DYNCREATE(CPropPageAbout, CPropertyPage)

CPropPageAbout::CPropPageAbout() : PropPageBase(CPropPageAbout::IDD)
{
	//{{AFX_DATA_INIT(CPropPageAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPropPageAbout::~CPropPageAbout()
{
}


void CPropPageAbout::DoDataExchange(CDataExchange* pDX)
{
	PropPageBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageAbout)
	DDX_Control(pDX, IDC_EDIT_KEY, m_edit_key);
	DDX_Control(pDX, IDC_EDIT_URL1, m_edit_url1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropPageAbout, CPropertyPage)
	//{{AFX_MSG_MAP(CPropPageAbout)
	ON_BN_CLICKED(IDC_BUTTON_VALIDATE_KEY, &CPropPageAbout::OnBnClickedButtonValidateKey)
	ON_BN_CLICKED(IDC_BUTTON_START_TRIAL, &CPropPageAbout::OnBnClickedButtonGetTrialKey)
	//}}AFX_MSG_MAP
   ON_EN_SETFOCUS(IDC_EDIT_KEY, &CPropPageAbout::OnEnSetfocusEditKey)
   ON_BN_CLICKED(IDC_BUTTON_PURCHASE_LICENSE, &CPropPageAbout::OnBnClickedButtonPurchaseLicense)
   ON_EN_CHANGE(IDC_EDIT_KEY, &CPropPageAbout::OnEnChangeEditKey)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageAbout message handlers


BOOL CPropPageAbout::OnInitDialog() 
{
	PropPageBase::OnInitDialog();
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	unsigned char mac[8];
	if(mainWnd && mainWnd->GetLicense().GetAdapterMac(mac))
	{
		CString txt;
		txt.Format(TXT("%02X-%02X-%02X-%02X-%02X-%02X"), 
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		GetDlgItem(IDC_STATIC_MY_MAC)->SetWindowText(txt);
	}

	// TODO: Add extra initialization here
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPropPageAbout::OnBnClickedButtonValidateKey()
{
	char_t typed_key[1024];
	m_edit_key.UpdateData(TRUE);
	m_edit_key.GetWindowText(typed_key, sizeof(typed_key)/sizeof(char_t));

	m_input_handler->OnButtonKey(typed_key);
}

void CPropPageAbout::OnBnClickedButtonGetTrialKey()
{
	unsigned char mac[8];
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	if(mainWnd && mainWnd->GetLicense().GetAdapterMac(mac))
	{
		STRING url;
		url.sprintf(TXT("http://www.osletek.com/trial2186f%02X-%02X-%02X-%02X-%02X-%02Xm"),
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		int len = url.size() * sizeof(wchar_t);
		wchar_t *path = new wchar_t[len + 16];
		memset(path, 0, len + 16);
		memcpy(path, url.s(), len);
		CreateThread(0, 0, mainWnd->ShellExecuteThread, (void*)path, 0, 0);
	}
}


void CPropPageAbout::OnBnClickedButtonPurchaseLicense()
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	if(mainWnd)
	{
		STRING url;
		url.sprintf(TXT("https://www.osletek.com/?a=order&fileid=2186"));
		int len = url.size() * sizeof(wchar_t);
		wchar_t *path = new wchar_t[len + 16];
		memset(path, 0, len + 16);
		memcpy(path, url.s(), len);
		CreateThread(0, 0, mainWnd->ShellExecuteThread, (void*)path, 0, 0);
	}
}

void CPropPageAbout::OnEnSetfocusEditKey()
{
   // Needed to auto select the contents of the key edit box
}

void CPropPageAbout::OnEnChangeEditKey()
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   CString key;
   GetDlgItem(IDC_EDIT_KEY)->GetWindowText(key);
   if(key.GetLength() > 0)
   {
      GetDlgItem(IDC_BUTTON_START_TRIAL)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_BUTTON_VALIDATE_KEY)->ShowWindow(SW_SHOW);
   }
   else
   {
      if(mainWnd->IsLicenseValid())
         GetDlgItem(IDC_BUTTON_START_TRIAL)->ShowWindow(SW_SHOW);
      else
         GetDlgItem(IDC_BUTTON_START_TRIAL)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_BUTTON_VALIDATE_KEY)->ShowWindow(SW_HIDE);
   }
}
