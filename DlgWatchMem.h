#if !defined(AFX_DLGWATCHMEM_H__B94EE84B_520D_4EA5_99FF_3BC5A8B02DEF__INCLUDED_)
#define AFX_DLGWATCHMEM_H__B94EE84B_520D_4EA5_99FF_3BC5A8B02DEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWatchMem.h : header file
//

#include "dlgWatchBase.h"

class DEVICE;

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchMem dialog

class CDlgWatchMem : public CDlgWatchBase
{
// Construction
public:
	CDlgWatchMem(DEVICE* device, const STRING &email_to, CWnd* pParent = NULL);   // standard constructor


// Dialog Data
	//{{AFX_DATA(CDlgWatchMem)
	enum { IDD = IDD_MEMORY_WATCH };
	CEdit	m_edit_time;
	CEdit	m_edit_mem;
	CEdit	m_edit_emailto;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWatchMem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgWatchMem)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGWATCHMEM_H__B94EE84B_520D_4EA5_99FF_3BC5A8B02DEF__INCLUDED_)
