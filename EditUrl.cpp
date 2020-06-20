// EditUrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "EditUrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BG_COLOR 0x00FFFFFF
/////////////////////////////////////////////////////////////////////////////
// CEditUrl

CEditUrl::CEditUrl()
{
	m_hCursor = (HCURSOR)LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_FINGER));
	DWORD bgcolor = GetSysColor(COLOR_WINDOW);
	m_Brush.CreateSolidBrush(bgcolor);
}

CEditUrl::~CEditUrl()
{
	m_Brush.DeleteObject();
}

BEGIN_MESSAGE_MAP(CEditUrl, CEdit)
	//{{AFX_MSG_MAP(CEditUrl)
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditUrl message handlers

void CEditUrl::OnSetFocus(CWnd* pOldWnd) 
{
}

void CEditUrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
}

BOOL CEditUrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	SetCursor(m_hCursor);	
	return TRUE;
}

void CEditUrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	wchar_t *url = new wchar_t[MAX_PATH];
	
	GetWindowText(url, MAX_PATH);

	CreateThread(0, 0, ShellExecuteThread, url, 0, 0);

	CEdit::OnLButtonUp(nFlags, point);
}

DWORD CEditUrl::ShellExecuteThread(LPVOID lpParameter)
{
	wchar_t *url = (wchar_t*) lpParameter;

	ShellExecute(NULL, L"open", url, NULL,NULL, SW_SHOWNORMAL);

	delete url;
	return 1;
}


HBRUSH CEditUrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CEdit::OnCtlColor(pDC, pWnd, nCtlColor);
	
	pDC->SetTextColor(0x00ff0000);
	//pDC->SetBkColor(BG_COLOR);
	
	return hbr;
}

BOOL CEditUrl::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	HDC hdcChild = (HDC)wParam;
	SetTextColor(hdcChild, 0xFF0000);
	
	// Set the text background:
	SetBkColor(hdcChild, BG_COLOR);
	
	// Send what would have been the return value of OnCtlColor() - the
	// brush handle - back in pLResult:
	if(pLResult)
		*pLResult = (LRESULT)(m_Brush.GetSafeHandle());
	
	// Return TRUE to indicate that the message was handled:
	return TRUE;
}
