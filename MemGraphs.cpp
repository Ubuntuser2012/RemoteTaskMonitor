
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

IMPLEMENT_DYNCREATE(MemGraphs, CPropPageGraphs)

MemGraphs::MemGraphs()
	: CPropPageGraphs(MemGraphs::IDD)
	, m_total_mem(1)
{
	//{{AFX_DATA_INIT(CPropPageGraphs)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


MemGraphs::~MemGraphs()
{
}

void MemGraphs::DoDataExchange(CDataExchange* pDX)
{
	CPropPageGraphs::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CpuGraphs)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(MemGraphs, CPropPageGraphs)
	//{{AFX_MSG_MAP(CPropPageGraphs)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
END_MESSAGE_MAP()

void MemGraphs::CreateGraph(int xscale, COLORREF color, DWORD total_mem)
{
	PLOT_ITEM *plot_item = new PLOT_ITEM;
	m_graph_id = (int)plot_item;
	GetGraphWnd()->AddGraph(m_graph_id, color, 100);
	GetGraphWnd()->SetDeltaX(xscale);
	m_total_mem = total_mem;
}
void MemGraphs::CreateSplitter()
{
	CPropPageGraphs::CreateSplitter(RUNTIME_CLASS(CGraphWnd2), RUNTIME_CLASS(LegendCtrl));
	GetLegend()->AddColumn(L"Process");
	GetLegend()->AddColumn(L"PID");
}

int MemGraphs::GetLegendTitles(STRING *title, DEVICE *device, PLOT_ITEM* plot_item)
{
	PROCESS *p = device->getProcessById(plot_item->pid);
	if (p)
	{
		title[0] = p->getName();
		title[1].sprintf(TXT("%i"), p->getId());
		return 2;
	}
	return 0;
}
CGraphWnd2* MemGraphs::GetGraphWnd()
{
	return (CGraphWnd2*)m_splitFrame->GetSplitterWnd().GetPane(0, 0);
}
 void MemGraphs::UpdateGraphs(unsigned int x, const DEVICE *device)
{
	 ITERATOR it;
	 CGraph *g = (CGraph *)GetGraphWnd()->GetFirst(it);
	 while (g)
	 {
		 int id = g->GetId();

		 if (id == m_graph_id)
		 {
			 int percent = (int)device->memPercent();
			 if (percent > 100)
			 {
				 percent = 100;
			 }
			 GetGraphWnd()->AddPoint(id, x, percent);
		 }
		 else
		 {
			 PLOT_ITEM* plot_item = (PLOT_ITEM*)id;
			 if (plot_item->pid && !plot_item->tid)
			 {
				 PROCESS* process = device->getProcessById(plot_item->pid);
				 if (process)
				 {
					 int percent = process->memoryUsed() * 100 / m_total_mem;
					 GetGraphWnd()->AddPoint(id, x, percent);
				 }
			 }
			 break;
		 }
		 g = (CGraph *)GetGraphWnd()->GetNext(it);
	 }
 }