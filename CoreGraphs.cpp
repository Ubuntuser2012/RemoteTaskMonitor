
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

#include "CoreGraphs.h"

IMPLEMENT_DYNCREATE(CoreGraphs, CPropertyPage)

CoreGraphs::CoreGraphs()
	: PropPageBase(CoreGraphs::IDD)
	, m_graphBox(0)
	, m_num_graphs(0)
	, m_graph_id(0)
{
	//{{AFX_DATA_INIT(CoreGraphs)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CoreGraphs::~CoreGraphs()
{
	if (m_graphBox)
	{
		delete[] m_graphBox;
		m_num_graphs = 0;
	}
}

void CoreGraphs::DoDataExchange(CDataExchange* pDX)
{
	PropPageBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CoreGraphs)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CoreGraphs, PropPageBase)
	//{{AFX_MSG_MAP(PropPageBase)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
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

void CoreGraphs::Size()
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

void CoreGraphs::Refresh()
{
	for (int i = 0; i < m_num_graphs; ++i)
	{
		m_graphBox[i].Refresh();
	}
}

void CoreGraphs::UpdateGraphs(unsigned int x, const DEVICE *device)
{
	for (int i = 0; i < m_num_graphs; ++i)
	{
		if (i == 0)
		{
			m_graphBox[i].UpdateGraph(x, -1, device->cpuPercent(0));
		}
		else
		{
			m_graphBox[i].UpdateGraph(x, i - 1, device->cpuPercent(0, i - 1));
		}
	}
}

void CoreGraphs::CreateGraph(int xscale, COLORREF color, int count)
{
	if (count < 1)
	{
		return;
	}

	m_num_graphs = count > 1 ? count + 1 : count;
	m_graphBox = new GraphBox[m_num_graphs];

	for (int i = 0; i < m_num_graphs; ++i)
	{
		PLOT_ITEM *plot_item = new PLOT_ITEM;
		if (i == 0)
		{
			m_graph_id = (int)plot_item;
		}
		m_graphBox[i].Create(IDD_GRAPH_BOX, this);
		m_graphBox[i].CreateGraph((int)(plot_item),  xscale, color);
	}
}

 int CoreGraphs::GetGraphId() const
 {
	 return m_graph_id;
 }

 CGraphWnd2 *CoreGraphs::GetGraphWnd()
 {
	 return (m_graphBox && m_num_graphs > 0) ? m_graphBox[0].GetGraphWnd() : 0;
 }