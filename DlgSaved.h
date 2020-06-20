#if !defined(AFX_DLGSAVED_H__2B431111_EDB2_486D_A560_0464AD909082__INCLUDED_)
#define AFX_DLGSAVED_H__2B431111_EDB2_486D_A560_0464AD909082__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSaved.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSaved dialog

class CDlgSaved : public CDialog
{
// Construction
public:
	CDlgSaved(STRING &msg, STRING &fname, bool prompt_show_report, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSaved)
	enum { IDD = IDD_DIALOG_SAVED };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSaved)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	STRING m_msg;
	STRING m_fname;
	bool m_prompt_show_report;

	// Generated message map functions
	//{{AFX_MSG(CDlgSaved)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckDoNotPromptAgain();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSAVED_H__2B431111_EDB2_486D_A560_0464AD909082__INCLUDED_)
