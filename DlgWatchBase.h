#if !defined(AFX_DlgWatchBase_H__B94EE84B_520D_4EA5_99FF_3BC5A8B02DEF__INCLUDED_)
#define AFX_DlgWatchBase_H__B94EE84B_520D_4EA5_99FF_3BC5A8B02DEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWatchBase.h : header file
//

class DEVICE;

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchBase dialog

class CDlgWatchBase : public CDialog
{
// Construction
public:
	CDlgWatchBase(int IDD, DEVICE* device, const STRING &email_to, CWnd* pParent = NULL);   // standard constructor

// Implementation
protected:
	virtual BOOL OnInitDialog();
   BOOL EmailSetupOk();

	STRING m_email_to;
	DEVICE *m_device;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DlgWatchBase_H__B94EE84B_520D_4EA5_99FF_3BC5A8B02DEF__INCLUDED_)
