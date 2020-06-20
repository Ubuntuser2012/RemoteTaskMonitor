#if !defined(AFX_DLGDBGPS2_H__A81642E7_E3EF_45F7_99FE_1AC06995A93E__INCLUDED_)
#define AFX_DLGDBGPS2_H__A81642E7_E3EF_45F7_99FE_1AC06995A93E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgDbgPs2.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPs2 dialog

class CDlgDbgPs2 : public CDialog
{
// Construction
public:
	CDlgDbgPs2(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgDbgPs2)
	enum { IDD = IDD_DEBUG_PROCESS2 };
	CEdit	m_edit_map_file_path;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDbgPs2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgDbgPs2)
	virtual void OnOk();
   virtual void OnCancel();
   afx_msg void OnButtonMapFilePath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDBGPS2_H__A81642E7_E3EF_45F7_99FE_1AC06995A93E__INCLUDED_)
