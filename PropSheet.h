#if !defined(AFX_PROPSHEET_H__F092B8EA_045C_4594_A0B7_415F64852992__INCLUDED_)
#define AFX_PROPSHEET_H__F092B8EA_045C_4594_A0B7_415F64852992__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropSheet.h : header file
//

#include "../ds/ds.h"

/////////////////////////////////////////////////////////////////////////////
// CPropSheet

class CPropSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CPropSheet)

// Construction
public:
	CPropSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CPropSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

	void SetActiveTabAndLock(int nPage);
   void SetActivePage(int nPage);
   void UnlockTabs();
   void LockTabs();
   // Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropSheet)
	public:
	virtual BOOL OnInitDialog();
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropSheet)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL m_enable_selchanging_check;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPSHEET_H__F092B8EA_045C_4594_A0B7_415F64852992__INCLUDED_)
