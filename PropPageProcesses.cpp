// PropPageProcesses.cpp : implementation file
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
// CPropPageProcesses property page

IMPLEMENT_DYNCREATE(CPropPageProcesses, CPropertyPage)

CPropPageProcesses::CPropPageProcesses() : PropPageBase(CPropPageProcesses::IDD)
{
	//{{AFX_DATA_INIT(CPropPageProcesses)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPropPageProcesses::~CPropPageProcesses()
{
}

void CPropPageProcesses::DoDataExchange(CDataExchange* pDX)
{
	PropPageBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageProcesses)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropPageProcesses, PropPageBase)
	//{{AFX_MSG_MAP(CPropPageProcesses)
   ON_BN_CLICKED(IDC_CHECK_SHOW_THREADS, OnCheckShowThreads)
   ON_BN_CLICKED(IDC_CHECK_SHOW_MEMORY, OnCheckShowMemory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageProcesses message handlers
void CPropPageProcesses::OnCheckShowThreads() 
{
   m_input_handler->OnCheckShowThreads();
}

void CPropPageProcesses::OnCheckShowMemory()
{
   bool state = ((CButton*)GetDlgItem(IDC_CHECK_SHOW_MEMORY))->GetCheck() == 1 ? true:false;
   m_input_handler->OnCheckShowMemory(state);
}

void CPropPageProcesses::SetCheckShowMemory(bool state)
{
   ((CButton*)GetDlgItem(IDC_CHECK_SHOW_MEMORY))->SetCheck(state);
   m_input_handler->OnCheckShowMemory(state);
}

bool CPropPageProcesses::GetCheckShowMemory()
{
   bool state = ((CButton*)GetDlgItem(IDC_CHECK_SHOW_MEMORY))->GetCheck() == 1 ? true:false;   
   return state;
}
