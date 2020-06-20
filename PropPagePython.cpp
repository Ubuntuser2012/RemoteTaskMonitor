// PropPagePython.cpp : implementation file
//

#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "resource.h"
#include "PropPageBase.h"
#include "EditUrl.h"
#include "PropPagePython.h"

int py_main(const char_t *pyfile);

// CPropPagePython dialog

IMPLEMENT_DYNAMIC(CPropPagePython, CPropertyPage)

CPropPagePython::CPropPagePython()
	: PropPageBase(CPropPagePython::IDD)
{

}

CPropPagePython::~CPropPagePython()
{
}

void CPropPagePython::DoDataExchange(CDataExchange* pDX)
{
	PropPageBase::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_EDIT_URL_PYTHON, m_edit_url1);
}


BEGIN_MESSAGE_MAP(CPropPagePython, PropPageBase)
   ON_BN_CLICKED(IDC_BUTTON_LOAD_PY, &CPropPagePython::OnBnClickedButtonLoadPy)
   ON_BN_CLICKED(IDC_BUTTON_RUN_PY, &CPropPagePython::OnBnClickedButtonRunPy)
   ON_BN_CLICKED(IDC_CHECK_RUN_PY_AT_STARTUP, &CPropPagePython::OnBnClickedCheckRunPyAtStartup)
END_MESSAGE_MAP()


// CPropPagePython message handlers

BOOL CPropPagePython::OnInitDialog() 
{
	PropPageBase::OnInitDialog();
	
	// TODO: Add extra initialization here
   m_edit_url1.SetWindowText(L"https://www.osletek.com/pg/2204");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CPropPagePython::OnBnClickedButtonLoadPy()
{
	static char_t BASED_CODE g_szFilter[] = L"Python scripts (*.py)|*.py|Text files (*.txt)|*.txt|All Files (*.*)|*.*||";
	CFileDialog m_fileDialog(TRUE, 0, 0, OFN_ENABLESIZING|OFN_PATHMUSTEXIST|OFN_NONETWORKBUTTON, //|OFN_ALLOWMULTISELECT
		g_szFilter);

	char_t dir[MAX_DIR_PATH];
	if(GetCurrentDirectory(sizeof(dir)/sizeof(wchar_t), dir))
	{
		m_fileDialog.m_ofn.lpstrInitialDir=dir;
	}

	if(IDOK==m_fileDialog.DoModal())
	{
      CString pathName=m_fileDialog.GetPathName();
      GetDlgItem(IDC_EDIT_PY)->SetWindowText((LPCTSTR)pathName);
      m_path.set(pathName);
	}
	else
	{
	}
}

void CPropPagePython::EnableRunButton(bool enable)
{
   CButton* btn = (CButton*)GetDlgItem(IDC_BUTTON_RUN_PY);
   if(btn)
   {
      btn->EnableWindow(enable);
   }
}

void CPropPagePython::EnableCheckAutoStart(bool enable)
{
   CButton* chkbox = (CButton*)GetDlgItem(IDC_CHECK_RUN_PY_AT_STARTUP);
   if(chkbox)
   {
      chkbox->EnableWindow(enable);
      if(!enable)
         chkbox->SetCheck(BST_UNCHECKED);
   }
}

void CPropPagePython::OnBnClickedButtonRunPy()
{
   CString pathName;
   GetDlgItem(IDC_EDIT_PY)->GetWindowText(pathName);
   m_path.set(pathName);
   py_main(m_path.s());
}

void CPropPagePython::OnBnClickedCheckRunPyAtStartup()
{
   // TODO: Add your control notification handler code here
}

void CPropPagePython::SetPythonScript(const char_t* script)
{
   if(script)
   {
      int len = strlenW(script);
      if(len>0)
      {
         m_path.set(script);
         GetDlgItem(IDC_EDIT_PY)->SetWindowText(script);
         UpdateData(FALSE);
      }
   }
}

const STRING &CPropPagePython::GetPythonScript()
{
	UpdateData(FALSE);
   CString pathName;
   GetDlgItem(IDC_EDIT_PY)->GetWindowText(pathName);
   m_path.set(pathName);
   return m_path;
}

void CPropPagePython::SetRunPythonScriptAtStartup(bool set)
{
   CButton* btn = (CButton*)GetDlgItem(IDC_CHECK_RUN_PY_AT_STARTUP);
   btn->SetCheck(set ? BST_CHECKED:BST_UNCHECKED);
   UpdateData(TRUE);
}

bool CPropPagePython::GetRunPythonScriptAtStartup()
{
	UpdateData(FALSE);
   CButton* btn = (CButton*)GetDlgItem(IDC_CHECK_RUN_PY_AT_STARTUP);
   return (BST_CHECKED == btn->GetCheck()) ? true:false;
}


void CPropPagePython::Size() 
{
	CWnd *wndOutput=0, *wndPathBox;
	RECT rectMonitor, rectOutput, rectPathBox;

	wndOutput = GetDlgItem(IDC_EDIT_PY_OUTPUT);
	wndOutput->GetWindowRect(&rectOutput);

   wndPathBox = GetDlgItem(IDC_STATIC_PY_PATH);
	wndPathBox->GetWindowRect(&rectPathBox);

	if(wndOutput && wndPathBox)
	{
		GetWindowRect(&rectMonitor);
		int width = rectMonitor.right-rectMonitor.left - 35;
		int height = rectMonitor.bottom - rectPathBox.bottom - 35;
		ScreenToClient(&rectMonitor);
		ScreenToClient(&rectPathBox);
		int y = rectPathBox.bottom + 10;
		int x = rectMonitor.left + 10;
		wndOutput->SetWindowPos(0, x, y, width, height, 0);
	}
}
void CPropPagePython::Clear()
{
   CEdit *ed = (CEdit *)GetDlgItem(IDC_EDIT_PY_OUTPUT);
   if(ed)
   {
      ed->Clear();
      ed->SetWindowText(L"");
   }
}

#define MAX_EDIT_BOX_LINES 100
void CPropPagePython::Print(const char* msg)
{
   CEdit *ed = (CEdit *)GetDlgItem(IDC_EDIT_PY_OUTPUT);
   if(ed)
   {
      CString txt;
      CString newTxt(msg);
      //newTxt+=L"\n";
      
      int lineCount = ed->GetLineCount();
      if(lineCount > MAX_EDIT_BOX_LINES)
      {//trim the top line
         int start2ndline = ed->LineIndex(1);
         ed->SetSel(0, start2ndline, TRUE);
         ed->ReplaceSel(TEXT(""));
      }
      ed->SetSel(0,-1);// select all text and move cursor at the end
      ed->SetSel(-1);//  remove selection

      ed->ReplaceSel(newTxt);
   }
}
