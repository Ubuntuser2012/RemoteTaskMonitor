#pragma once
#include "PropPageGraphs.h"

class PropPageMemGraphs : public PropPageGraphs
{
public:
	DECLARE_DYNCREATE(PropPageMemGraphs)

	PropPageMemGraphs();
	~PropPageMemGraphs();
	void CreateGraph(int xscale, COLORREF color);
	virtual void CreateSplitter(std::function<void(const PLOT_DATA *plot_item)>OnToggleGenReport);
	virtual void UpdateGraphs(unsigned int x, const DEVICE *device);
	virtual GraphWnd2* GetGraphWnd();
	BOOL AddGraph(DEVICE *device, PLOT_DATA *plot_item);
	void Size();

	// Dialog Data
	//{{AFX_DATA(CpuGraphs)
	enum { IDD = IDD_MEM_GRAPH };
	//}}AFX_DATA
	
protected:
	// Overrides
	virtual int GetLegendTitles(STRING *title, DEVICE *device, PLOT_DATA* plot_item);
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CpuGraphs)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation

	// Generated message map functions
	//{{AFX_MSG(CpuGraphs)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
};

