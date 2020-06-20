#if !defined(AFX_DLGWATCHCPUTOTALALERT_H__B3B0BB66_E9BF_4620_89E8_B7F219CB54B9__INCLUDED_)
#define AFX_DLGWATCHCPUTOTALALERT_H__B3B0BB66_E9BF_4620_89E8_B7F219CB54B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWatchCPUTotalAlert.h : header file
//

class DEVICE;
/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPUTotalAlert dialog

class CDlgWatchCPUTotalAlert : public CDialog
{
// Construction
public:
	CDlgWatchCPUTotalAlert(DEVICE *device, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgWatchCPUTotalAlert)
	enum { IDD = IDD_TOTAL_CPU_WATCH_ALERT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWatchCPUTotalAlert)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	DEVICE *m_device;
	// Generated message map functions
	//{{AFX_MSG(CDlgWatchCPUTotalAlert)
   virtual void OnOK();
   virtual void OnCancel();
	afx_msg void OnDestroy();
	afx_msg void OnCancelThisWatch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGWATCHCPUTOTALALERT_H__B3B0BB66_E9BF_4620_89E8_B7F219CB54B9__INCLUDED_)
