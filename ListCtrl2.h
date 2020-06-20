#if !defined(AFX_LISTCTRL2_H__E7DBCFF7_580A_4885_B79F_A90667FEC957__INCLUDED_)
#define AFX_LISTCTRL2_H__E7DBCFF7_580A_4885_B79F_A90667FEC957__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListCtrl2.h : header file
//

typedef struct LIST_COLUMN_
{
	int id;
	int fmt;
	int width;
	int image;
	wchar_t name[32];
}
LIST_COLUMN;


enum LIST_COLUMN_ID//keep this enum in sync with const LIST_COLUMN CListCtrl2::m_col[LIST_COLUMN_MAX]={
{
	LIST_COLUMN_PROCESS,
	LIST_COLUMN_PID,
	LIST_COLUMN_CPU,
	LIST_COLUMN_CORE,
	LIST_COLUMN_PRIORITY,
	LIST_COLUMN_CPU_AVG,
	LIST_COLUMN_CPU_MAX,
	LIST_COLUMN_MEMORY,
	LIST_COLUMN_MEMORY_MAX,
	LIST_COLUMN_MAX
};

class CDlgRemoteCPU;

/////////////////////////////////////////////////////////////////////////////
// CListCtrl2 window

class CListCtrl2 : public CListCtrl
{
	// Construction
public:
	CListCtrl2();

	// Attributes
public:

	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrl2)
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CListCtrl2();
	BOOL Create(CDlgRemoteCPU *input_handler, CWnd *parent);
	void Size();
	void SetColumnTitle(int ncol, char_t *title);
	void EnableMenuItem(UINT id, BOOL enable);

	// Generated message map functions
protected:
	//{{AFX_MSG(CListCtrl2)
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnMenuItemProcessKill();
	afx_msg void OnMenuItemProcessStart();
	afx_msg void OnMenuItemProcessClone();
	afx_msg void OnMenuItemProcessRestart();
	afx_msg void OnMenuItemThreadKill();
	afx_msg void OnMenuItemThreadSuspend();
	afx_msg void OnMenuItemThreadResume();
	afx_msg void OnMenuItemThreadPriority();
	afx_msg void OnMenuItemCPUWatch();
	afx_msg void OnMenuItemTotalCPUWatch();
	afx_msg void OnMenuItemPlotCpuGraph();
	afx_msg void OnMenuItemPlotMemGraph();
	afx_msg void OnMenuItemCrashReport();
	afx_msg void OnMemoryWatch();
	afx_msg void OnMenuItemSelectCore();
	//}}AFX_MSG
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	void SetFont();
	void SetMenu();
	int MousePosToListIndex();
	void HandleMenu(int id);
	void SetSelectedThreadPriorities(const LIST &ids);
	void StartCPUWatch(const LIST &ids);
	void StartCPUWatch();
	void StartMemoryWatch();
	void StartRemoteProcess();
	void DebugRemoteProcess(PROCESS *p);

	CImageList m_ImageList;
	CDlgRemoteCPU *m_input_handler;
	CWnd *m_parent;
	CFont m_fontBold;
	HMENU m_menu_ps;
	bool m_erase_bg;
	bool m_pid_in_hex;
	static const LIST_COLUMN m_col[LIST_COLUMN_MAX];


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCTRL2_H__E7DBCFF7_580A_4885_B79F_A90667FEC957__INCLUDED_)
