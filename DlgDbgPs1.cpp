// DlgDbgPs1.cpp : implementation file
//

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "RemoteCPU.h"
#include "DlgDbgPs1.h"
#include "DlgWatchCfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPs1 dialog


CDlgDbgPs1::CDlgDbgPs1(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDbgPs1::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgDbgPs1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgDbgPs1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDbgPs1)
	DDX_Control(pDX, IDC_EDIT_PROCESS_PATH, m_edit_ps_path);
	DDX_Control(pDX, IDC_STATIC_PID, m_edit_pid);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDbgPs1, CDialog)
	//{{AFX_MSG_MAP(CDlgDbgPs1)
	ON_BN_CLICKED(IDC_RADIO_DEBUG_ACTIVE_PROCESS, OnRadioDebugActiveProcess)
	ON_BN_CLICKED(IDC_RADIO_DEBUG_NEW_PROCESS, OnRadioDebugNewProcess)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPs1 message handlers

void CDlgDbgPs1::OnRadioDebugActiveProcess() 
{
	// TODO: Add your control notification handler code here
	CDlgWatchCfg* parent = (CDlgWatchCfg*)GetParent();
   parent->OnRadioDebugActiveProcess();
}

void CDlgDbgPs1::OnRadioDebugNewProcess() 
{
	// TODO: Add your control notification handler code here
   CDlgWatchCfg* parent = (CDlgWatchCfg*)GetParent();
   parent->OnRadioDebugNewProcess();
}

void CDlgDbgPs1::OnOk() 
{
}

void CDlgDbgPs1::OnCancel() 
{
	CDlgWatchCfg* parent = (CDlgWatchCfg*)GetParent();
	parent->OnCancel();
}
