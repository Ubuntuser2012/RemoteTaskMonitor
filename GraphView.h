#pragma once

#include <functional>

class STRING;
class DEVICE;
class GraphWnd2;

// GraphView view

class GraphView : public CView
{
	DECLARE_DYNAMIC(GraphView)

protected:
	GraphView();           // protected constructor used by dynamic creation
	virtual ~GraphView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void Draw() = 0;
	virtual void Size() = 0;
	virtual void UpdateGraphs(unsigned int x, const DEVICE *device) = 0;
	virtual BOOL AddGraph(PLOT_DATA *plot_item, COLORREF color, const STRING *legend_titles, int num_legend_titles) = 0;
	virtual int CreateGraph(int xscale, COLORREF color, int num_graphs) = 0;
	virtual GraphWnd2 *GetGraphWnd() = 0;
		virtual void SetCallbacks(
		std::function<void(const STRING *legend_titles, int num_legend_titles, PLOT_DATA *plot_item, COLORREF color)>OnAddGraphCb,
		std::function<void(int graph_id, const STRING *values, int num_values)>OnUpdateGraphCb);

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	std::function<void(const STRING *legend_titles, int num_legend_titles, PLOT_DATA *plot_item, COLORREF color)>OnAddGraph;
	std::function<void(int graph_id, const STRING *values, int num_values)>OnUpdateGraph;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void StartDrawTimer();
};


