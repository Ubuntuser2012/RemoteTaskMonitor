#pragma once

// PropPageGraphs.h : header file
//

#include "PropPageBase.h"
#include "SplitterFrame.h"
#include "LegendCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CPropPageGraphs dialog

class PropPageGraphs : public PropPageBase
{
	DECLARE_DYNAMIC(PropPageGraphs)

// Construction
public:
	PropPageGraphs(UINT nIDTemplate, UINT nIDCaption = 0);
	~PropPageGraphs();

	BOOL AddGraph(DEVICE * device, PLOT_DATA * plot_item, COLORREF color = 0);
	virtual GraphWnd2* GetGraphWnd() = 0;
	LegendCtrl* GetLegend();
	virtual void CreateSplitter(CRuntimeClass* leftView, CRuntimeClass* rightView);
	int GetGraphId() const;
	void SetGraphId(int graph_id);
	virtual void Size();
	virtual int GetLegendTitles(STRING *title, DEVICE *device, PLOT_DATA* plot_item) = 0;
	virtual void UpdateGraphs(unsigned int x, const DEVICE *device) = 0;

// Implementation
protected:
	BOOL GraphExists(PLOT_DATA *plot_item);
	COLORREF RandomColor();
	void OnRemoveGraph(const PLOT_DATA *plot_item);
	void OnChangeGraphColor(const PLOT_DATA *plot_item, COLORREF color);
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CpuGraphs)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CPropPageGraphs)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	SplitterFrame *m_splitFrame;
	int m_graph_id;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
