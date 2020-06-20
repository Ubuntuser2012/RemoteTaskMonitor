#if !defined(AFX_PROPPAGEPROCESSES_H__946511A6_CAE5_4AB1_82B6_5F8B601B8F93__INCLUDED_)
#define AFX_PROPPAGEPROCESSES_H__946511A6_CAE5_4AB1_82B6_5F8B601B8F93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropPageProcesses.h : header file
//

#include "PropPageBase.h"
/////////////////////////////////////////////////////////////////////////////
// CPropPageProcesses dialog

class CPropPageProcesses : public PropPageBase
{
	DECLARE_DYNCREATE(CPropPageProcesses)

// Construction
public:
	CPropPageProcesses();
	~CPropPageProcesses();

// Dialog Data
	//{{AFX_DATA(CPropPageProcesses)
	enum { IDD = IDD_PROCESSES };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


   void SetCheckShowMemory(bool state);
   bool GetCheckShowMemory();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageProcesses)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageProcesses)
   afx_msg void OnCheckShowThreads();
   afx_msg void OnCheckShowMemory(); 
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPPAGEPROCESSES_H__946511A6_CAE5_4AB1_82B6_5F8B601B8F93__INCLUDED_)
