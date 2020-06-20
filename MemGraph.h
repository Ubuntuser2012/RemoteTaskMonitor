#pragma once
#include "GraphView.h"
#include "GraphBox.h"


// MemGraph view

class MemGraph : public GraphView
{
	DECLARE_DYNCREATE(MemGraph)

protected:
	MemGraph();           // protected constructor used by dynamic creation
	virtual ~MemGraph();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	void Size();
	void Draw();
	void UpdateGraphs(unsigned int x, const DEVICE *device);
	BOOL AddGraph(PLOT_DATA *plot_item, COLORREF color, const STRING *legend_titles, int num_legend_titles);
	int CreateGraph(int xscale, COLORREF color, int num_graphs);
	GraphWnd2 *GetGraphWnd();

protected:
	DECLARE_MESSAGE_MAP()

	GraphBox *m_graphBox;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


