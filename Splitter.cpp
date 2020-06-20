// Splitter.cpp : implementation file
//

#include "stdafx.h"
#include "Splitter.h"
#include "LegendCtrl.h"
#include "GraphView.h"

#include "GraphWnd2.h"

#include "../common/util.h"
// Splitter

IMPLEMENT_DYNAMIC(Splitter, CSplitterWnd)

Splitter::Splitter()
	: m_leftPaneWidthPercent(80)
	, m_initialized(false)

{

}

Splitter::~Splitter()
{
}


BEGIN_MESSAGE_MAP(Splitter, CSplitterWnd)
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// Splitter message handlers
void Splitter::SetInitialized()
{
	m_initialized = true;
}

void Splitter::CreateSplitter(CWnd* parentWnd, CRuntimeClass* leftView, CRuntimeClass* rightView, const SIZE &sz, CCreateContext* pContext)
{
	BOOL ret;
	ret = CreateStatic(parentWnd, 1, 2, WS_CHILD | WS_VISIBLE);

	ret = CreateView(0, 0, leftView,
		CSize(sz.cx * m_leftPaneWidthPercent / 100, sz.cy), pContext);
	ret = CreateView(0, 1, rightView,
		CSize(sz.cx * (100 - m_leftPaneWidthPercent) / 100, sz.cy), pContext);

	SetWindowPos(NULL, 0, 0, sz.cx, sz.cy, SWP_SHOWWINDOW);
}

void Splitter::Size()
{
	RECT rect;
	GetParent()->GetClientRect(&rect);
	SetWindowPos(NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
	SetRowColInfo(rect.right - rect.left, rect.bottom - rect.top);
}

void Splitter::SetRowColInfo(int cx, int cy)
{
	if (!m_initialized)
	{
		return;
	}
	
	SetRowInfo(0, cy, cy);
	SetColumnInfo(0, cx * m_leftPaneWidthPercent / 100, 0);
	SetColumnInfo(1, cx * (100 - m_leftPaneWidthPercent) / 100, 0);

	RecalcLayout();
	GetPane(0, 0)->RedrawWindow(0, 0, RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
	GetPane(0, 1)->RedrawWindow(0, 0, RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
}

void Splitter::OnSize(UINT nType, int cx, int cy)
{
	CSplitterWnd::OnSize(nType, cx, cy);
	SetRowColInfo(cx, cy);
}


void Splitter::OnLButtonUp(UINT nFlags, CPoint point)
{
	CSplitterWnd::OnLButtonUp(nFlags, point);

	int cx=0, cxMin=0;
	GetColumnInfo(0, cx, cxMin);

	RECT rect;
	GetClientRect(&rect);

	m_leftPaneWidthPercent = 100 * cx / (rect.right - rect.left);
	Size();
}
