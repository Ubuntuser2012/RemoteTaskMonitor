#if !defined(AFX_PROPPAGEABOUT_H__F25984C8_20EC_431F_AC31_36F3BE5CD564__INCLUDED_)
#define AFX_PROPPAGEABOUT_H__F25984C8_20EC_431F_AC31_36F3BE5CD564__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropPageAbout.h : header file
//

#include "PropPageBase.h"
#include "EditUrl.h"
/////////////////////////////////////////////////////////////////////////////
// CPropPageAbout dialog

class CPropPageAbout : public PropPageBase
{
	DECLARE_DYNCREATE(CPropPageAbout)

// Construction
public:
	CPropPageAbout();
	~CPropPageAbout();

// Dialog Data
	//{{AFX_DATA(CPropPageAbout)
	enum { IDD = IDD_ABOUT };
	CEdit	m_edit_key;
	CEditUrl m_edit_url1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageAbout)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageAbout)
	afx_msg void OnBnClickedButtonValidateKey();
	afx_msg void OnBnClickedButtonGetTrialKey();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
   afx_msg void OnEnSetfocusEditKey();
   afx_msg void OnBnClickedButtonPurchaseLicense();
   afx_msg void OnEnChangeEditKey();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPPAGEABOUT_H__F25984C8_20EC_431F_AC31_36F3BE5CD564__INCLUDED_)
