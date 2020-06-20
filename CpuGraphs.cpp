
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

#include "CpuGraphs.h"

IMPLEMENT_DYNCREATE(CpuGraphs, GraphView)

CpuGraphs::CpuGraphs()
	: m_graphBox(0)
	, m_num_graphs(0)
{
	//{{AFX_DATA_INIT(CoreGraphs)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CpuGraphs::~CpuGraphs()
{
	if (m_graphBox)
	{
		delete[] m_graphBox;
		m_num_graphs = 0;
	}
}

void CpuGraphs::DoDataExchange(CDataExchange* pDX)
{
	GraphView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CoreGraphs)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CpuGraphs, CView)
	//{{AFX_MSG_MAP(GraphView)
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_WM_SIZE()
END_MESSAGE_MAP()

static void CalculateRowsCols(int num_cores, int &rows, int &cols)
{
	rows = 1;
	if ((float)(num_cores / 2.0f) > rows)
	{
		rows++;
	}
	if (rows > 2)
	{
		rows = 2;
	}

	cols = 2;
	if (num_cores < 5)
	{
		cols = 2;
	}
	else
	{
		cols = num_cores / 2;
		if ((float)(num_cores / 2.0f) > cols)
		{
			cols++;
		}
	}
}

void CpuGraphs::Size()
{
	int rows, cols;
	CalculateRowsCols(m_num_graphs - 1, rows, cols);

	RECT rect;
	GetClientRect(&rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	int row = 0;
	int col = 0;
	int graph0_height = (m_num_graphs > 1) ? height * 3 / 5 : height;
	int core_graphs_height = height - graph0_height;

	for (int i = 0; i < m_num_graphs; ++i)
	{
		if (i == 0)
		{
			m_graphBox[i].SetWindowPos(0, 0, 0, width, graph0_height, SWP_SHOWWINDOW);
		}
		else
		{
			int x = col * (width / cols);
			int y = graph0_height + row * (core_graphs_height / rows);
			m_graphBox[i].SetWindowPos(0, x, y, width / cols, core_graphs_height / rows, SWP_SHOWWINDOW);
			if (col + 1 == cols)
			{
				row++;
			}
			col = (col + 1) % cols;
		}
		m_graphBox[i].Size();
	}
}

void CpuGraphs::Draw()
{
	for (int i = 0; i < m_num_graphs; ++i)
	{
		m_graphBox[i].Draw();
	}
}

void CpuGraphs::UpdateGraphs(unsigned int x, const DEVICE *device)
{
	for (int core = 0; core < m_num_graphs; ++core)
	{
		int graph_id = 0;
		if (core == 0)
		{
			ITERATOR it;
			bool first = true;
			for (PLOT *g = (PLOT *)m_graphBox[core].GetGraphWnd()->GetFirst(it); g;
				g = (PLOT *)m_graphBox[core].GetGraphWnd()->GetNext(it))
			{
				float percent = 0;
				int pid = 0;
				int tid = 0;
				int priority = 0;
				STRING process_name;
				STRING thread_name;

				if (first)
				{
					first = false;
					percent = device->cpuPercent(0);
					g->AddPoint(x, (int)percent);

					STRING title;
					title.sprintf(TXT("CPU %.02f%%"), percent);
					m_graphBox[core].SetTitle(title);
					process_name.set(TXT("Total"));
				}
				else
				{
					PLOT_DATA* plot_item = (PLOT_DATA*)g->GetId();
					if (plot_item->pid && !plot_item->tid)
					{
						PROCESS* process = device->getProcessById(plot_item->pid);
						if (process)
						{
							percent = process->cpuPercent(0);
							g->AddPoint(x, (int)percent);
							core = process->affinity();
							if (process->getName())
							{
								process_name.set(process->getName());
							}
						}
						else
						{
							g->AddPoint(x, 0);
						}
					}
					else if (plot_item->pid && plot_item->tid)
					{
						PROCESS* process = device->getProcessById(plot_item->pid);
						if (process)
						{
							THREAD* thread = process->getThreadById(plot_item->tid);
							if (thread)
							{
								percent = thread->cpuPercent(0);
								g->AddPoint(x, (int)percent);
								core = thread->affinity();
								tid = plot_item->tid;
								if (thread->getName())
								{
									thread_name.set(thread->getName());
								}
							}
							else
							{
								g->AddPoint(x, 0);
							}
							if (process->getName())
							{
								process_name.set(process->getName());
							}
						}
					}
					pid = plot_item->pid;
				}

				STRING values[10];
				int s = 0;
				if (process_name.size())
				{
					values[s].set(process_name);
				}
				s++;
				values[s].sprintf(TXT("%.02f"), percent);
				s++;
				if (core > 0)
				{
					values[s].sprintf(TXT("%i"), core + 1);
				}
				s++;
				if (pid > 0)
				{
					values[s].sprintf(TXT("%i"), pid);
				}
				s++;
				if (tid > 0)
				{
					values[s].sprintf(TXT("%i"), tid);
				}
				s++;
				if (priority > 0)
				{
					values[s].sprintf(TXT("%i"), priority);
				}
				s++;
				if (thread_name.size() > 0)
				{
					values[s].set(thread_name.s());
				}
				s++;
				OnUpdateGraph(g->GetId(), values, s);
			}
		}
		else
		{
			ITERATOR it;
			PLOT *g = (PLOT *)m_graphBox[core].GetGraphWnd()->GetFirst(it);
			float percent = device->cpuPercentBySummingCoreCpu(0, core - 1);
			g->AddPoint(x, (int)percent);
			STRING title;
			title.sprintf(TXT("Core %i - CPU %.02f%%"), core, percent);
			m_graphBox[core].SetTitle(title);
			STRING values[1];
			OnUpdateGraph(g->GetId(), values, 0);
		}
	}
}

BOOL CpuGraphs::AddGraph(PLOT_DATA *plot_item, COLORREF color, const STRING *legend_titles, int num_legend_titles)
{
	if (GetGraphWnd()->AddGraph((int)plot_item, color, 100))
	{
		OnAddGraph(legend_titles, num_legend_titles, plot_item, color);
		return TRUE;
	}
	return FALSE;
}

int CpuGraphs::CreateGraph(int xscale, COLORREF color, int count)
{
	int graph_id = 0;
	if (count < 1)
	{
		return 0;
	}

	m_num_graphs = count > 1 ? count + 1 : count;
	m_graphBox = new GraphBox[m_num_graphs];

	for (int i = 0; i < m_num_graphs; ++i)
	{
		PLOT_DATA *plot_item = new PLOT_DATA;
		m_graphBox[i].Create(IDD_GRAPH_BOX, this);
		m_graphBox[i].CreateGraph((int)(plot_item),  xscale, color);
		if (i == 0)
		{
			graph_id = (int)plot_item;
			STRING legend_titles[10];
			legend_titles[0].set(TXT("Total"));
			OnAddGraph(legend_titles, 10, plot_item, color);
		}
	}
	StartDrawTimer();
	return graph_id;
}

 GraphWnd2 *CpuGraphs::GetGraphWnd()
 {
	 return (m_graphBox && m_num_graphs > 0) ? m_graphBox[0].GetGraphWnd() : 0;
 }


 void CpuGraphs::OnSize(UINT nType, int cx, int cy)
 {
	 GraphView::OnSize(nType, cx, cy);
	 Size();

	 // TODO: Add your message handler code here
 }
