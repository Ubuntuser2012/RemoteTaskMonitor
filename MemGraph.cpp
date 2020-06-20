// MemGraph.cpp : implementation file
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

#include "MemGraph.h"


// MemGraph

IMPLEMENT_DYNCREATE(MemGraph, GraphView)

MemGraph::MemGraph()
	: m_graphBox(0)
{

}

MemGraph::~MemGraph()
{
	if (m_graphBox)
	{
		delete m_graphBox;
	}
}

BEGIN_MESSAGE_MAP(MemGraph, GraphView)
	ON_WM_TIMER()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// MemGraph drawing
void MemGraph::Size()
{
	RECT rect;
	GetClientRect(&rect);

	if (m_graphBox)
	{
		m_graphBox->SetWindowPos(0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
		m_graphBox->Size();
	}
}

void MemGraph::Draw()
{
	if (m_graphBox)
	{
		m_graphBox->Draw();
	}
}

void MemGraph::UpdateGraphs(unsigned int x, const DEVICE *device)
{
	ITERATOR it;
	bool first = true;
	for (PLOT *g = (PLOT *)m_graphBox->GetGraphWnd()->GetFirst(it); g;
		g = (PLOT *)m_graphBox->GetGraphWnd()->GetNext(it))
	{
		STRING ps_name;
		float percent = 0;
		int memory_used_bytes = 0;
		int pid = 0;
		if (first)
		{
			first = false;
			percent = device->memPercent();
			g->AddPoint(x, percent);

			STRING title;
			title.sprintf(TXT("Memory %.02f%%"), percent);
			m_graphBox->SetTitle(title);
			ps_name.set(TXT("Total"));
			if (device->getMemStatus())
			{
				memory_used_bytes = (device->getMemStatus()->totalMemKB - device->getMemStatus()->freeMemKB) * 1024;
				ps_name.sprintf(TXT("Total %s"), PROCESS::memoryUsedHumanReadable(device->getMemStatus()->totalMemKB * 1024).s());
			}
		}
		else
		{
			PLOT_DATA* plot_item = (PLOT_DATA*)g->GetId();
			if (plot_item->pid)
			{
				PROCESS* process = device->getProcessById(plot_item->pid);
				if(process && device->getMemStatus() && device->getMemStatus()->totalMemKB > 0)
				{
					percent = process->memoryUsed() * 100 / device->getMemStatus()->totalMemKB / 1024.0;
					g->AddPoint(x, percent);
					memory_used_bytes = process->memoryUsed();
				}
				if(process && process->getName() && *process->getName())
				{
					ps_name.set(process->getName());
				}
				pid = plot_item->pid;
			}
		}
		STRING values[10];
		int i = 0;
		values[i++].set(ps_name);
		values[i++].sprintf(TXT("%.02f"), percent);
		values[i++].set(PROCESS::memoryUsedHumanReadable(memory_used_bytes));
		values[i++].sprintf(TXT("%i"), pid);
		OnUpdateGraph(g->GetId(), values, i);
	}
}

BOOL MemGraph::AddGraph(PLOT_DATA *plot_item, COLORREF color, const STRING *legend_titles, int num_legend_titles)
{
	if (m_graphBox->GetGraphWnd()->AddGraph((int)plot_item, color, 100))
	{
		OnAddGraph(legend_titles, num_legend_titles, plot_item, color);
		return TRUE;
	}
	return FALSE;
}

int MemGraph::CreateGraph(int xscale, COLORREF color, int num_graphs)
{
	m_graphBox = new GraphBox;

	PLOT_DATA *plot_item = new PLOT_DATA;
	m_graphBox->Create(IDD_GRAPH_BOX, this);
	m_graphBox->CreateGraph((int)(plot_item), xscale, color);
	STRING legend_titles[10];
	legend_titles[0].set(TXT("Total"));
	OnAddGraph(legend_titles, 10, plot_item, color);

	StartDrawTimer();
	return (int)plot_item;
}

GraphWnd2 *MemGraph::GetGraphWnd()
{
	return m_graphBox ? m_graphBox->GetGraphWnd() : 0;
}


void MemGraph::OnSize(UINT nType, int cx, int cy)
{
	GraphView::OnSize(nType, cx, cy);
	Size();
	// TODO: Add your message handler code here
}
