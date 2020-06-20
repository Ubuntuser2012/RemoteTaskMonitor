#if !defined(AFX_TABCTRL2_H__B5D2C73C_37C1_4968_87AC_CD3AA638E3C7__INCLUDED_)
#define AFX_TABCTRL2_H__B5D2C73C_37C1_4968_87AC_CD3AA638E3C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabCtrl2.h : header file
//

typedef struct TabPage_
{
	int m_id;
	wchar_t m_name[32];
}
TabPage;

enum TABPAGE
{
	TABPAGE_DEVICE,
	TABPAGE_PROCESSES,
	TABPAGE_GRAPHS,
	TABPAGE_ABOUT,
	TABPAGES_TOTAL,
   TABPAGE_FILTERS,
   TABPAGE_EVENTS,
};

class CDlgRemoteCPU;

/////////////////////////////////////////////////////////////////////////////
// CTabCtrl2 window

class CTabCtrl2 : public CTabCtrl
{
// Construction
public:
	CTabCtrl2();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabCtrl2)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTabCtrl2();
	BOOL Create(CDlgRemoteCPU *parent);
	void Size();
	void ShowPageDevices(bool show);
	void ShowPageProcesses(bool show);
	void ShowPageGraphs(bool show);
   void ShowPageAbout(bool show);
	void ShowPage(int pageNum);
   void SelectTab(int page);

	// Generated message map functions
protected:
	//{{AFX_MSG(CTabCtrl2)
   afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	CDlgRemoteCPU *m_wndParent;
	TabPage m_tabPage[TABPAGES_TOTAL];

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABCTRL2_H__B5D2C73C_37C1_4968_87AC_CD3AA638E3C7__INCLUDED_)
