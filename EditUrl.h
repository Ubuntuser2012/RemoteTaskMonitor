#if !defined(AFX_URLEDCTRL_H__809D06A3_DE10_44FE_9232_B500ABE123E3__INCLUDED_)
#define AFX_URLEDCTRL_H__809D06A3_DE10_44FE_9232_B500ABE123E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditUrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditUrl window

class CEditUrl : public CEdit
{
// Construction
public:
	CEditUrl();

public:
// Attributes

public:
// Operations
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditUrl)
	public:
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	//}}AFX_VIRTUAL

public:
// Implementation
	virtual ~CEditUrl();

protected:
// Attributes
	CBrush m_Brush;
	HCURSOR m_hCursor;

	static DWORD WINAPI ShellExecuteThread(LPVOID lpParameter);

// Generated message map functions
	//{{AFX_MSG(CEditUrl)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_URLEDCTRL_H__809D06A3_DE10_44FE_9232_B500ABE123E3__INCLUDED_)
