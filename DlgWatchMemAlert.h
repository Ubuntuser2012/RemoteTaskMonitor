#if !defined(AFX_DlgWatchMemAlert_H__E244748A_E781_46F5_A605_F208D45CA379__INCLUDED_)
#define AFX_DlgWatchMemAlert_H__E244748A_E781_46F5_A605_F208D45CA379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWatchMemAlert.h : header file
//

class DEVICE;

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchMemAlert dialog

class CDlgWatchMemAlert : public CDialog
{
// Construction
public:
	CDlgWatchMemAlert(DEVICE * device, CWnd* pParent = NULL);   // standard constructor
   virtual ~CDlgWatchMemAlert();

   // Dialog Data
	//{{AFX_DATA(CDlgWatchMemAlert)
	enum { IDD = IDD_MEMORY_WATCH_ALERT };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWatchMemAlert)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   DEVICE *m_device;

   // Generated message map functions
	//{{AFX_MSG(CDlgWatchMemAlert)
   virtual void OnOK();
   virtual void OnCancel();
   virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnCancelAll();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DlgWatchMemAlert_H__E244748A_E781_46F5_A605_F208D45CA379__INCLUDED_)
