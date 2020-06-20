#if !defined(AFX_DlgWatchCPU_H__B94EE84B_520D_4EA5_99FF_3BC5A8B02DEF__INCLUDED_)
#define AFX_DlgWatchCPU_H__B94EE84B_520D_4EA5_99FF_3BC5A8B02DEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWatchCPU.h : header file
//

#include "dlgWatchBase.h"

class DEVICE;


/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPU dialog

class CDlgWatchCPU : public CDlgWatchBase
{
// Construction
public:
	CDlgWatchCPU(const LIST &ids, DEVICE* device, const STRING &email_to, CWnd* pParent = NULL);   // standard constructor
   static void StartCPUWatch(EXECOBJ *execobj, DEVICE *device, float max_cpu, DWORD pid, DWORD tid, int duration, const char_t *email_to, bool reset_device, bool kill_ps, bool restart_ps);

   // Dialog Data
	//{{AFX_DATA(CDlgWatchCPU)
	enum { IDD = IDD_CPU_WATCH };
	CEdit	m_edit_cpu;
	CEdit	m_edit_time;
	CEdit	m_edit_emailto;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWatchCPU)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   const LIST &m_ids;

	// Generated message map functions
	//{{AFX_MSG(CDlgWatchCPU)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DlgWatchCPU_H__B94EE84B_520D_4EA5_99FF_3BC5A8B02DEF__INCLUDED_)
