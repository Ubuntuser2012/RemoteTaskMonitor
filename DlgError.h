#if !defined(AFX_DLGERROR_H__8C0176A7_A1E9_4CB4_93D5_772B46937B81__INCLUDED_)
#define AFX_DLGERROR_H__8C0176A7_A1E9_4CB4_93D5_772B46937B81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgError.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgError dialog

class CDlgError : public CDialog
{
// Construction
public:
	CDlgError(CWnd* pParent = NULL);   // standard constructor
	void SetMessage(STRING &title, STRING &msg, STRING &url);

// Dialog Data
	//{{AFX_DATA(CDlgError)
	enum { IDD = IDD_ERROR };
	CEditUrl m_edit_url1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgError)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgError)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGERROR_H__8C0176A7_A1E9_4CB4_93D5_772B46937B81__INCLUDED_)
