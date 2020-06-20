#if !defined(AFX_DlgTrialValidation_H__8C0176A7_A1E9_4CB4_93D5_772B46937B81__INCLUDED_)
#define AFX_DlgTrialValidation_H__8C0176A7_A1E9_4CB4_93D5_772B46937B81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgTrialValidation.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgTrialValidation dialog

class CDlgTrialValidation : public CDialog
{
// Construction
public:
	CDlgTrialValidation(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgTrialValidation)
	enum { IDD = IDD_TRIAL_VALIDATION };
   bool m_use;
	CString m_host;
   CString m_port;
   CString m_user;
   CString m_pass;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgTrialValidation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   RECT m_rect;

   void MakeSmaller();
   void MakeBigger();

	// Generated message map functions
	//{{AFX_MSG(CDlgTrialValidation)
	afx_msg void OnButtonUseProxy();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DlgTrialValidation_H__8C0176A7_A1E9_4CB4_93D5_772B46937B81__INCLUDED_)
