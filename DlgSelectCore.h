#pragma once


// DlgSelectCore dialog

class DlgSelectCore : public CDialog
{
	DECLARE_DYNAMIC(DlgSelectCore)

public:
	DlgSelectCore(const LIST &ids, int num_cores, CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgSelectCore();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SELECT_CORE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(DlgSelectCore)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	const LIST &m_ids;
	int m_num_cores;

};
