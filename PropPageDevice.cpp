// PropPageDevice.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "afxbutton.h"
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
#include "PropPageDevice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageDevice property page

IMPLEMENT_DYNCREATE(CPropPageDevice, CPropertyPage)

CPropPageDevice::CPropPageDevice() : PropPageBase(CPropPageDevice::IDD)
{
	//{{AFX_DATA_INIT(CPropPageDevice)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPropPageDevice::~CPropPageDevice()
{
}

void CPropPageDevice::DoDataExchange(CDataExchange* pDX)
{
	PropPageBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageDevice)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropPageDevice, PropPageBase)
	//{{AFX_MSG_MAP(CPropPageDevice)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_SNAPSHOT, &CPropPageDevice::OnButtonSaveSnapshot)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CPropPageDevice::OnButtonResetDevice)
	ON_BN_CLICKED(IDC_BUTTON_REPORT, &CPropPageDevice::OnButtonGenerateReport)
	ON_BN_CLICKED(IDC_BUTTON_CONN, &CPropPageDevice::OnButtonConnect)
	ON_CBN_SELENDOK(IDC_COMBO_CEMONS, &CPropPageDevice::OnComboCemonsChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageDevice message handlers

void CPropPageDevice::OnButtonGenerateReport()
{
	// TODO: Add your control notification handler code here
	m_input_handler->OnGenerateReport(true);
}

void CPropPageDevice::OnButtonResetDevice() 
{
	// TODO: Add your control notification handler code here
	m_input_handler->OnButtonResetDevice();
}

void CPropPageDevice::OnButtonSaveSnapshot() 
{
	// TODO: Add your control notification handler code here
	m_input_handler->OnButtonSaveSnapshot();	
}

void CPropPageDevice::OnComboCemonsChanged()
{
//	m_input_handler->OnComboCemonsChanged();
}

void CPropPageDevice::OnButtonConnect()
{
	m_input_handler->OnConnectButtonClicked();
}

BOOL CPropPageDevice::OnInitDialog() 
{
	PropPageBase::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
