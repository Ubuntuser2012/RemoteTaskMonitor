// DlgDbgPs2.cpp : implementation file
//

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "RemoteCPU.h"
#include "DlgDbgPs2.h"
#include "DlgWatchCfg.h"
#include "DlgDbgPsInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPs2 dialog


CDlgDbgPs2::CDlgDbgPs2(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDbgPs2::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgDbgPs2)
	//}}AFX_DATA_INIT
}


void CDlgDbgPs2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDbgPs2)
	DDX_Control(pDX, IDC_EDIT_MAP_FILE_PATH, m_edit_map_file_path);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDbgPs2, CDialog)
	//{{AFX_MSG_MAP(CDlgDbgPs2)
   ON_BN_CLICKED(ID_GET_MAP_FILE, OnButtonMapFilePath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPs2 message handlers

void CDlgDbgPs2::OnOk() 
{
}

void CDlgDbgPs2::OnCancel() 
{
	CDlgWatchCfg* parent = (CDlgWatchCfg*)GetParent();
	parent->OnCancel();
}

void CDlgDbgPs2::OnButtonMapFilePath()
{
   CString path;
   if(CDlgDbgPsInfo::getMapFilePath(path))
   {
      CWnd * editpath = GetDlgItem(IDC_EDIT_MAP_FILE_PATH);
      editpath->SetWindowText(path);
   }
}