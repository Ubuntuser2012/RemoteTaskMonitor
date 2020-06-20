#pragma once
#include "afxwin.h"


// CPropPagePython dialog

class CPropPagePython : public PropPageBase
{
	DECLARE_DYNAMIC(CPropPagePython)

public:
	CPropPagePython();
	virtual ~CPropPagePython();

// Dialog Data
	enum { IDD = IDD_PYTHON };

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   CEditUrl m_edit_url1;
   STRING m_path;
	DECLARE_MESSAGE_MAP()
public:
	// Generated message map functions
	//{{AFX_MSG(CPropPagePython)
   afx_msg void OnBnClickedButtonLoadPy();
   afx_msg void OnBnClickedButtonRunPy();
   afx_msg void OnBnClickedCheckRunPyAtStartup();
	virtual BOOL OnInitDialog();
	//}}AFX_MGS

   void SetPythonScript(const char_t* script);
   const STRING &GetPythonScript();
   void SetRunPythonScriptAtStartup(bool set);
   bool GetRunPythonScriptAtStartup();
   void Size();
   void Print(const char* msg);
   void Clear();
   void EnableCheckAutoStart(bool enable);
   void EnableRunButton(bool enable);

};
