// PropPageGraphs.cpp : implementation file
//

#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageGraphs property page

IMPLEMENT_DYNAMIC(PropPageGraphs, CPropertyPage)

PropPageGraphs::PropPageGraphs(UINT nIDTemplate, UINT nIDCaption)
: PropPageBase(nIDTemplate)
, m_splitFrame(NULL)
, m_graph_id(0)
{
	//{{AFX_DATA_INIT(CPropPageGraphs)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

PropPageGraphs::~PropPageGraphs()
{

}

void PropPageGraphs::DoDataExchange(CDataExchange* pDX)
{
	PropPageBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropPageGraphs)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(PropPageGraphs, PropPageBase)
	//{{AFX_MSG_MAP(CPropPageGraphs)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageGraphs message handlers

void PropPageGraphs::CreateSplitter(CRuntimeClass* leftView, CRuntimeClass* rightView)
{
	CString strMyClass = AfxRegisterWndClass(CS_VREDRAW |
		CS_HREDRAW,
		::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH) ::GetStockObject(WHITE_BRUSH),
		::LoadIcon(NULL, IDI_APPLICATION));

	CRect rect;
	GetClientRect(&rect);

	m_splitFrame = new SplitterFrame(leftView, rightView);
	m_splitFrame->Create(strMyClass, L"", WS_CHILD|WS_VISIBLE, rect, this);
}

void PropPageGraphs::Size()
{
	m_splitFrame->Size();
}

COLORREF PropPageGraphs::RandomColor()
{
	int r = rand() % 0xff;
	int g = rand() % 0xff;
	int b = rand() % 0xff;
	COLORREF color = RGB(r, g, b);
	return color;
}

BOOL PropPageGraphs::AddGraph(DEVICE *device, PLOT_DATA *plot_item, COLORREF color)
{//TODO: remove this function!!
	BOOL ret = false;
	if (GraphExists(plot_item))
	{
		return false;
	}
	if (color == 0)
	{
		color = RandomColor();
	}
	int id = (int)plot_item;
	GetGraphWnd()->AddGraph(id, color, 100);
	if (device)
	{
		STRING titles[10];
		int num_titles = GetLegendTitles(titles, device, plot_item);
		ret = (GetLegend()->InsertItem(titles, num_titles, color, plot_item) > -1);
	}
	else
	{
		ret = (GetLegend()->InsertItem(TXT("Total"), color, plot_item) > -1);
	}
	return ret;
}

int PropPageGraphs::GetGraphId() const
{
	return m_graph_id;
}
void PropPageGraphs::SetGraphId(int graph_id)
{
	m_graph_id = graph_id;
}

BOOL PropPageGraphs::GraphExists(PLOT_DATA *plot_item)
{
	ITERATOR it;
	PLOT *gr = (PLOT *)GetGraphWnd()->GetFirst(it);
	while (gr)
	{
		PLOT_DATA *it1 = 0, *it2 = 0;
		it1 = (PLOT_DATA *)gr->GetId();
		it2 = plot_item;
		if (it1->pid == it2->pid && it1->tid == it2->tid)
		{
			return true;
		}
		gr = (PLOT *)GetGraphWnd()->GetNext(it);
	}
	return false;
}

LegendCtrl* PropPageGraphs::GetLegend()
{
	return (LegendCtrl*)m_splitFrame->GetSplitterWnd().GetPane(0, 1);
}

void PropPageGraphs::OnRemoveGraph(const PLOT_DATA *plot_item)
{
	GetGraphWnd()->RemoveGraph((int)plot_item);
}

void PropPageGraphs::OnChangeGraphColor(const PLOT_DATA *plot_item, COLORREF color)
{
	GetGraphWnd()->SetGraphColor((int)plot_item, color);
}
