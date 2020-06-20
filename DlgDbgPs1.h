#if !defined(AFX_DLGDBGPS1_H__B1213FAC_82C5_4C67_9544_9782FAF29DB6__INCLUDED_)
#define AFX_DLGDBGPS1_H__B1213FAC_82C5_4C67_9544_9782FAF29DB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgDbgPs1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPs1 dialog

class CDlgDbgPs1 : public CDialog
{
// Construction
public:
	CDlgDbgPs1(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgDbgPs1)
	enum { IDD = IDD_DEBUG_PROCESS1 };
	CEdit	m_edit_ps_path;
	CStatic	m_edit_pid;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDbgPs1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgDbgPs1)
	afx_msg void OnRadioDebugActiveProcess();
	afx_msg void OnRadioDebugNewProcess();
   virtual void OnCancel();
   virtual void OnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDBGPS1_H__B1213FAC_82C5_4C67_9544_9782FAF29DB6__INCLUDED_)
