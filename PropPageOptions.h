#if !defined(AFX_PROPPAGEOPTIONS_H__F1F042D0_FBB9_4F30_AAB4_8F83DAE201BB__INCLUDED_)
#define AFX_PROPPAGEOPTIONS_H__F1F042D0_FBB9_4F30_AAB4_8F83DAE201BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropPageOptions.h : header file
//

#include "PropPageBase.h"
/////////////////////////////////////////////////////////////////////////////
// CPropPageOptions dialog

class CPropPageOptions : public PropPageBase
{
	DECLARE_DYNCREATE(CPropPageOptions)

// Construction
public:
	CPropPageOptions();
	~CPropPageOptions();

   void SetCheckCaptureOnDev(bool check);
   void SetCheckDontPromptShowReport(bool chk_state);


   void GetEmailTo(STRING &out);
   void GetSMTPServer(STRING &out);
   short GetSMTPPort();

   void SetEmailTo(const char_t *s);
   void SetSMTPServer(const char_t *s);
   void SetSMTPPort(const char_t* s);

   void SendEmail(const STRING &to, const STRING &subject, const STRING &body);

// Dialog Data
	//{{AFX_DATA(CPropPageOptions)
	enum { IDD = IDD_OPTIONS };
	CEdit	m_edit_emailto;
	CEdit	m_edit_smtp_server;
	CEdit	m_edit_smpt_port;
	//CEdit	m_edit_cemon_priority;
	CEdit	m_edit_snap_interval;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageOptions)
	afx_msg void OnButtonCaptureStart();
	afx_msg void OnButtonCaptureStop();
	afx_msg void OnButtonSetCemonPriority();
	afx_msg void OnButtonEmailTest();
	afx_msg void OnCheckCaptureOnDev();
	afx_msg void OnCheckDontPromptShowReport();
	afx_msg void OnButtonLoadCapturedData();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPPAGEOPTIONS_H__F1F042D0_FBB9_4F30_AAB4_8F83DAE201BB__INCLUDED_)
