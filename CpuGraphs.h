#pragma once
#include "GraphView.h"
#include "GraphBox.h"

class CpuGraphs : public GraphView
{
public:
	DECLARE_DYNCREATE(CpuGraphs)

	CpuGraphs();
	~CpuGraphs();
	virtual void UpdateGraphs(unsigned int x, const DEVICE *device);
	int CreateGraph(int xscale, COLORREF color, int count);
	BOOL AddGraph(PLOT_DATA *plot_item, COLORREF color, const STRING *legend_titles, int num_legend_titles);
	void Size();
	void Draw();
	GraphWnd2 *GetGraphWnd();

	// Dialog Data
	//{{AFX_DATA(CpuGraphs)
	enum { IDD = IDD_CORE_GRAPHS };
	//}}AFX_DATA
	
protected:
	// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CoreGraphs)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
	// Generated message map functions
	//{{AFX_MSG(CpuGraphs)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	GraphBox *m_graphBox;
	int m_num_graphs;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

