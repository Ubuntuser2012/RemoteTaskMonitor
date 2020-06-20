#pragma once
#include "PropPageBase.h"
#include "GraphBox.h"

class CoreGraphs : public PropPageBase
{
public:
	DECLARE_DYNCREATE(CoreGraphs)

	CoreGraphs();
	~CoreGraphs();
	virtual void UpdateGraphs(unsigned int x, const DEVICE *device);
	void CreateGraph(int xscale, COLORREF color, int count);
	void Size();
	void Refresh();
	int GetGraphId() const;
	CGraphWnd2 *GetGraphWnd();

	// Dialog Data
	//{{AFX_DATA(CpuGraphs)
	enum { IDD = IDD_CORE_GRAPH };
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
	int m_graph_id;
};

