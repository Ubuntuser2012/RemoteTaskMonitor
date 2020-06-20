#if !defined(AFX_PROPPAGEDEVICE_H__0AC2A884_5110_4E58_826F_FCBEF0CBED3F__INCLUDED_)
#define AFX_PROPPAGEDEVICE_H__0AC2A884_5110_4E58_826F_FCBEF0CBED3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropPageDevice.h : header file
//

#include "PropPageBase.h"
/////////////////////////////////////////////////////////////////////////////
// CPropPageDevice dialog

class CPropPageDevice : public PropPageBase
{
	DECLARE_DYNCREATE(CPropPageDevice)

// Construction
public:
	CPropPageDevice();
	~CPropPageDevice();

// Dialog Data
	//{{AFX_DATA(CPropPageDevice)
	enum { IDD = IDD_DEVICE };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageDevice)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageDevice)
	afx_msg void OnButtonResetDevice();
	afx_msg void OnButtonSaveSnapshot();
	afx_msg void OnButtonGenerateReport();
	afx_msg void OnComboCemonsChanged();
	afx_msg void OnButtonConnect();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPPAGEDEVICE_H__0AC2A884_5110_4E58_826F_FCBEF0CBED3F__INCLUDED_)
