// GraphView.cpp : implementation file
//

#include "stdafx.h"
#include "../ds/ds.h"
extern "C" {
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "devicelist.h"
#include "remotecpu.h"
#include "dlgRemotecpu.h"
#include "../common/util.h"

#include "GraphView.h"


// GraphView

IMPLEMENT_DYNAMIC(GraphView, CView)

GraphView::GraphView()
{

}

GraphView::~GraphView()
{
}

BEGIN_MESSAGE_MAP(GraphView, CView)
END_MESSAGE_MAP()


// GraphView drawing

void GraphView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// GraphView diagnostics

#ifdef _DEBUG
void GraphView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void GraphView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// GraphView message handlers


void GraphView::SetCallbacks(
	std::function<void(const STRING *legend_titles, int num_legend_titles, PLOT_DATA *plot_item, COLORREF color)>OnAddGraphCb,
	std::function<void(int graph_id, const STRING *values, int num_values)>OnUpdateGraphCb)
{
	OnAddGraph = OnAddGraphCb;
	OnUpdateGraph = OnUpdateGraphCb;
}

void GraphView::StartDrawTimer()
{
	const int GRAPH_REDRAW_TIMEOUT = 200;
	SetTimer(1, GRAPH_REDRAW_TIMEOUT, NULL);
}

void GraphView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		Draw();
	}
	CView::OnTimer(nIDEvent);
}
