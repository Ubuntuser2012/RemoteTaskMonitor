#if !defined(AFX_DLGDBGPS3_H__93E744EB_4B55_433E_8E75_9DC47085278D__INCLUDED_)
#define AFX_DLGDBGPS3_H__93E744EB_4B55_433E_8E75_9DC47085278D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgDbgPs3.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPs3 dialog

class CDlgDbgPs3 : public CDialog
{
// Construction
public:
	CDlgDbgPs3(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgDbgPs3)
	enum { IDD = IDD_DEBUG_PROCESS3 };
	CEdit	m_edit_ps_path;
	CEdit	m_edit_email_to;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDbgPs3)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgDbgPs3)
	afx_msg void OnCheckSilentAfterCrash();
	afx_msg void OnCheckRestartAfterCrash();
	afx_msg void OnCheckPsWatch();
	afx_msg void OnCheckResetDevice();
	virtual void OnOk();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDBGPS3_H__93E744EB_4B55_433E_8E75_9DC47085278D__INCLUDED_)
