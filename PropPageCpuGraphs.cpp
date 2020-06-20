
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

#include "PropPageCpuGraphs.h"

IMPLEMENT_DYNCREATE(PropPageCpuGraphs, PropPageGraphs)

PropPageCpuGraphs::PropPageCpuGraphs()
	: PropPageGraphs(PropPageCpuGraphs::IDD)
{
	//{{AFX_DATA_INIT(CPropPageGraphs)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

PropPageCpuGraphs::~PropPageCpuGraphs()
{
	for (int i = 0; i < m_graph_data.GetCount(); ++i)
	{
		PROP_PAGE_GRAPH_DATA *data = (PROP_PAGE_GRAPH_DATA*)m_graph_data.GetAt(i);
		delete data;
	}
}

void PropPageCpuGraphs::DoDataExchange(CDataExchange* pDX)
{
	PropPageGraphs::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CpuGraphs)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(PropPageCpuGraphs, PropPageGraphs)
	//{{AFX_MSG_MAP(CPropPageGraphs)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
END_MESSAGE_MAP()

enum LEGEND_TITLE
{
	LEGEND_TITLE_PROCESS_NAME=0,
	LEGEND_TITLE_CPU_PERCENT,
	LEGEND_TITLE_CORE,
	LEGEND_TITLE_PID,
	LEGEND_TITLE_TID,
	LEGEND_TITLE_PRIORITY,
	LEGEND_TITLE_THREAD_NAME,
	LEGEND_TITLE_TOTAL
};

void PropPageCpuGraphs::AddLegendTitles()
{
	for (int i = 0; i < LEGEND_TITLE_TOTAL; i++)
	{
		switch (i)
		{
		case LEGEND_TITLE_PROCESS_NAME: GetLegend()->AddColumn(L"Process"); break;
		case LEGEND_TITLE_CPU_PERCENT: GetLegend()->AddColumn(L"CPU %"); break;
		case LEGEND_TITLE_CORE: GetLegend()->AddColumn(L"Core"); break;
		case LEGEND_TITLE_PID: GetLegend()->AddColumn(L"PID"); break;
		case LEGEND_TITLE_TID: GetLegend()->AddColumn(L"TID"); break;
		case LEGEND_TITLE_PRIORITY: GetLegend()->AddColumn(L"Priority"); break;
		case LEGEND_TITLE_THREAD_NAME: GetLegend()->AddColumn(L"Thread"); break;
		}
	}
}

void PropPageCpuGraphs::CreateSplitter(std::function<void(const PLOT_DATA *plot_item)>OnEndReport)
{
	PropPageGraphs::CreateSplitter(RUNTIME_CLASS(CpuGraphs), RUNTIME_CLASS(LegendCtrl));
	CpuGraphs* cpu_graphs = (CpuGraphs*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	cpu_graphs->SetCallbacks(
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
		UpdateGraphColor(plot_item, color);
	});
	AddLegendTitles();
}

bool PropPageCpuGraphs::HasGraph(const STRING &name, int &idx)
{
	idx = -1;
	for (int i = 0; i < m_graph_data.GetCount(); ++i)
	{
		PROP_PAGE_GRAPH_DATA *data = (PROP_PAGE_GRAPH_DATA*)m_graph_data.GetAt(i);
		if (data->m_name.strcmp(name.s()) == 0)
		{
			idx = i;
			return true;
		}
	}
	return false;
}
bool PropPageCpuGraphs::HasGraph(DWORD pid, int &idx)
{
	idx = -1;
	if (pid == 0)
	{
		return false;
	}
	for (int i = 0; i < m_graph_data.GetCount(); ++i)
	{
		PROP_PAGE_GRAPH_DATA *data = (PROP_PAGE_GRAPH_DATA*)m_graph_data.GetAt(i);
		if (data->m_pid == pid)
		{
			idx = i;
			return true;
		}
	}
	return false;
}
COLORREF PropPageCpuGraphs::RecallGraphColor(const DEVICE *device, const PLOT_DATA *plot_item, COLORREF color)
{
	PROCESS *p = device->getProcessById(plot_item->pid);
	if (!p)
	{
		return color;
	}
	int idx = -1;
	if (HasGraph(p->getName(), idx))
	{
		PROP_PAGE_GRAPH_DATA *data = (PROP_PAGE_GRAPH_DATA*)m_graph_data.GetAt(idx);
		color = data->m_color;
	}
	if (!HasGraph(p->getId(), idx))
	{
		logg(TXT("InsertItem color 0x%x pid %i tid %i, %s\n"),
			color, plot_item->pid, plot_item->tid, p->getName());
		PROP_PAGE_GRAPH_DATA *data = new PROP_PAGE_GRAPH_DATA(plot_item->pid, plot_item->tid, p->getName(), color);
		m_graph_data.Insert(data);
	}
	return color;
}

void PropPageCpuGraphs::UpdateGraphColor(const PLOT_DATA *plot_item, COLORREF color)
{
	logg(TXT("RememberGraphColor color 0x%x pid %i tid %i\n"), color, plot_item->pid, plot_item->tid);
	for (int i = 0; i < m_graph_data.GetCount(); ++i)
	{
		PROP_PAGE_GRAPH_DATA *data = (PROP_PAGE_GRAPH_DATA*)m_graph_data.GetAt(i);
		if (data->m_pid == plot_item->pid && data->m_tid == plot_item->tid)
		{
			data->m_color = color;
			break;
		}
	}
}

int PropPageCpuGraphs::GetLegendTitles(STRING *title, DEVICE *device, PLOT_DATA* plot_item)
{
	int num_titles = 0;
	PROCESS *p = device->getProcessById(plot_item->pid);
	if (p)
	{
		title[LEGEND_TITLE_PROCESS_NAME] = p->getName();
		title[LEGEND_TITLE_CPU_PERCENT].sprintf(TXT(""));
		title[LEGEND_TITLE_CORE].sprintf(TXT("%i"), p->affinity());
		title[LEGEND_TITLE_PID].sprintf(TXT("%i"), p->getId());
		num_titles = 4;

		if (plot_item->tid)
		{
			THREAD *t = p->getThreadById(plot_item->tid);
			if (t)
			{
				title[LEGEND_TITLE_TID].sprintf(TXT("%i"), t->getId());
				title[LEGEND_TITLE_PRIORITY].sprintf(TXT("%i"), t->getPriority());
				title[LEGEND_TITLE_CORE].sprintf(TXT("%i"), t->affinity());
				num_titles += 3;
				if (t->getName() && t->getName()[0])
				{
					title[LEGEND_TITLE_THREAD_NAME].sprintf(TXT("%i"), t->getName());
					num_titles++;
				}
			}
		}
	}
	return num_titles;
}
GraphWnd2* PropPageCpuGraphs::GetGraphWnd()
{
	CpuGraphs* cpu_graphs = (CpuGraphs*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	return cpu_graphs ? cpu_graphs->GetGraphWnd() : 0;
}

void PropPageCpuGraphs::CreateGraph(int xscale, COLORREF color, int count)
{
	CpuGraphs* cpu_graphs = (CpuGraphs*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	m_graph_id = cpu_graphs->CreateGraph(xscale, color, count);
}

BOOL PropPageCpuGraphs::AddGraph(DEVICE *device, PLOT_DATA *plot_item)
{
	CpuGraphs* cpu_graphs = (CpuGraphs*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	COLORREF color = RandomColor();
	color = RecallGraphColor(device, plot_item, color);

	STRING titles[10];
	int num_titles = 0;
	if (device)
	{
		num_titles = GetLegendTitles(titles, device, plot_item);
	}
	return cpu_graphs->AddGraph(plot_item, color, titles, num_titles);
}

void PropPageCpuGraphs::UpdateGraphs(unsigned int x, const DEVICE *device)
{
	CpuGraphs* cpu_graphs = (CpuGraphs*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	cpu_graphs->UpdateGraphs(x, device);
}

void PropPageCpuGraphs::Size()
{
	PropPageGraphs::Size();
	CpuGraphs* cpu_graphs = (CpuGraphs*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
	cpu_graphs->Size();
	m_splitFrame->Size();
}