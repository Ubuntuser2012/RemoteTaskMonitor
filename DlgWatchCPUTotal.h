#if !defined(AFX_DLGWATCHCPUTOTAL_H__2E479755_80B8_4C4D_8963_5590E3B13DB9__INCLUDED_)
#define AFX_DLGWATCHCPUTOTAL_H__2E479755_80B8_4C4D_8963_5590E3B13DB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWatchCPUTotal.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPUTotal dialog

class CDlgWatchCPUTotal : public CDlgWatchBase
{
// Construction
public:
	CDlgWatchCPUTotal(DEVICE * device, const STRING &email_to, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgWatchCPUTotal)
	enum { IDD = IDD_TOTAL_CPU_WATCH };
	CEdit	m_edit_time;
	CEdit	m_edit_emailto;
	CEdit	m_edit_cpu;
	//}}AFX_DATA

// Implementation
   static void StartCPUWatch(DEVICE *device, float max_cpu, int duration, const char_t *email_to, bool reset_device);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWatchCPUTotal)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgWatchCPUTotal)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGWATCHCPUTOTAL_H__2E479755_80B8_4C4D_8963_5590E3B13DB9__INCLUDED_)
