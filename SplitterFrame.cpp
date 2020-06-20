// SplitterFrame.cpp : implementation file
//

#include "stdafx.h"
#include "SplitterFrame.h"

static const float GRAPH_WIDTH_RATIO = 0.8f;
static const float LEGEND_WIDTH_RATIO = 0.2f;

// SplitterFrame

IMPLEMENT_DYNAMIC(SplitterFrame, CFrameWnd)

SplitterFrame::SplitterFrame(CRuntimeClass* leftView, CRuntimeClass* rightView)
	: m_initialized(false)
	, m_leftView(leftView)
	, m_rightView(rightView)
{

}

SplitterFrame::~SplitterFrame()
{
}


BEGIN_MESSAGE_MAP(SplitterFrame, CFrameWnd)
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()


const Splitter &SplitterFrame::GetSplitterWnd()
{
	return m_splitter;
}


BOOL SplitterFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	SIZE sz;
	sz.cx = lpcs->cx;
	sz.cy = lpcs->cy;

	m_splitter.CreateSplitter(this, m_leftView, m_rightView, sz, pContext);

	return TRUE;//	CFrameWnd::OnCreateClient(lpcs, pContext);
}

int SplitterFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_splitter.SetInitialized();
	return 0;
}

void SplitterFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	if (nType != SIZE_MINIMIZED)
	{
		m_splitter.SetRowColInfo(cx, cy);
	}
}

void SplitterFrame::Size()
{
	RECT rect;
	GetParent()->GetClientRect(&rect);
	SetWindowPos(NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);

	m_splitter.Size();
}