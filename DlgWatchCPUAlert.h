#if !defined(AFX_DlgWatchCPUAlert_H__E244748A_E781_46F5_A605_F208D45CA379__INCLUDED_)
#define AFX_DlgWatchCPUAlert_H__E244748A_E781_46F5_A605_F208D45CA379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWatchCPUAlert.h : header file
//

class DEVICE;

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPUAlert dialog

class CDlgWatchCPUAlert : public CDialog
{
// Construction
public:
	CDlgWatchCPUAlert(DEVICE * device, WATCH_ITEM *watch_item, CWnd* pParent = NULL);   // standard constructor
   virtual ~CDlgWatchCPUAlert();

   // Dialog Data
	//{{AFX_DATA(CDlgWatchCPUAlert)
	enum { IDD = IDD_CPU_WATCH_ALERT };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWatchCPUAlert)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   DEVICE *m_device;
   WATCH_ITEM *m_watch_item;

   // Generated message map functions
	//{{AFX_MSG(CDlgWatchCPUAlert)
   virtual void OnOK();
   virtual void OnCancel();
   virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnCancelAll();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCancelThisWatch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DlgWatchCPUAlert_H__E244748A_E781_46F5_A605_F208D45CA379__INCLUDED_)
