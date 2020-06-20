#include "stdafx.h"
#include "../common/types.h"
extern "C" {
#include "comm.h"
}
#include "../ds/ds.h"
#include "resource.h"
#include "DlgStartProcess.h"

char_t CDlgStartProcess::m_ps_path[MAX_RFILE_PATH];

CDlgStartProcess::CDlgStartProcess(CWnd* pParent /*=NULL*/)
: CDialog(CDlgStartProcess::IDD, pParent)
{

}

void CDlgStartProcess::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CDlgStartProcess)
   DDX_Control(pDX, IDC_EDIT_PROCESS_PATH, m_edit_path);
   //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgStartProcess, CDialog)
   //{{AFX_MSG_MAP(CDlgStartProcess)
   ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CDlgStartProcess::OnOK() 
{
   m_edit_path.UpdateData(TRUE);
   m_edit_path.GetWindowText(m_ps_path, sizeof(m_ps_path)/sizeof(char_t));
   CDialog::OnOK();
}

BOOL CDlgStartProcess::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
      if(m_ps_path)
      {
         m_edit_path.SetWindowTextW(m_ps_path);
      }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
