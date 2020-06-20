
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
#include "PropPageMemGraphs.h"

IMPLEMENT_DYNCREATE(PropPageMemGraphs, PropPageGraphs)

PropPageMemGraphs::PropPageMemGraphs()
	: PropPageGraphs(PropPageMemGraphs::IDD)
{
	//{{AFX_DATA_INIT(CPropPageGraphs)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


PropPageMemGraphs::~PropPageMemGraphs()
{
}

void PropPageMemGraphs::DoDataExchange(CDataExchange* pDX)
{
	PropPageGraphs::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MemGraphs)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(PropPageMemGraphs, PropPageGraphs)
	//{{AFX_MSG_MAP(CPropPageGraphs)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
END_MESSAGE_MAP()


void PropPageMemGraphs::CreateGraph(int xscale, COLORREF color)
{
	MemGraph* mem_graph = (MemGraph*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	m_graph_id = mem_graph->CreateGraph(xscale, color, 1);
}

void PropPageMemGraphs::CreateSplitter(std::function<void(const PLOT_DATA *plot_item)>OnEndReport)
{
	PropPageGraphs::CreateSplitter(RUNTIME_CLASS(MemGraph), RUNTIME_CLASS(LegendCtrl));
	MemGraph* mem_graph = (MemGraph*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	mem_graph->SetCallbacks(
		[this](const STRING *legend_titles, int num_legend_titles, PLOT_DATA *plot_item, COLORREF color)->void {
		GetLegend()->InsertItem(legend_titles, num_legend_titles, color, plot_item);
	},
		[this](int graph_id, const STRING *values, int num_values)->void {
		GetLegend()->UpdateItem(graph_id, values, num_values);
	});
	GetLegend()->SetCallbacks(
		[this, OnEndReport](const PLOT_DATA *plot_item)->void {
		OnRemoveGraph(plot_item);
		OnEndReport(plot_item);
	},
		[this](const PLOT_DATA *plot_item, COLORREF color)->void {
		OnChangeGraphColor(plot_item, color);
	});
	GetLegend()->AddColumn(L"Process");
	GetLegend()->AddColumn(L"Memory %");
	GetLegend()->AddColumn(L"Bytes");
	GetLegend()->AddColumn(L"PID");
}

BOOL PropPageMemGraphs::AddGraph(DEVICE *device, PLOT_DATA *plot_item)
{
	MemGraph* mem_graphs = (MemGraph*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	COLORREF color = RandomColor();
	STRING titles[10];
	int num_titles = 0;
	if (device)
	{
		num_titles = GetLegendTitles(titles, device, plot_item);
	}
	return mem_graphs->AddGraph(plot_item, color, titles, num_titles);
}

int PropPageMemGraphs::GetLegendTitles(STRING *title, DEVICE *device, PLOT_DATA* plot_item)
{
	PROCESS *p = device->getProcessById(plot_item->pid);
	if (p)
	{
		int i = 0;
		title[i++] = p->getName();
		float percent = device->getMemStatus()->totalMemKB > 0 ?
			p->memoryUsed() * 100 / (float)device->getMemStatus()->totalMemKB / 1024.0 : 0;
		title[i++].sprintf(TXT("%.02f"), percent);
		title[i++].sprintf(TXT("%i"), p->memoryUsed());
		title[i++].sprintf(TXT("%i"), p->getId());
		return i;
	}
	return 0;
}
GraphWnd2* PropPageMemGraphs::GetGraphWnd()
{
	MemGraph* mem_graphs = (MemGraph*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	return mem_graphs->GetGraphWnd();
}

void PropPageMemGraphs::UpdateGraphs(unsigned int x, const DEVICE *device)
{
	MemGraph* mem_graphs = (MemGraph*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	mem_graphs->UpdateGraphs(x, device);
}


void PropPageMemGraphs::Size()
{
	PropPageGraphs::Size();
	MemGraph* mem_graphs = (MemGraph*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	mem_graphs->Size();
	m_splitFrame->Size();
}