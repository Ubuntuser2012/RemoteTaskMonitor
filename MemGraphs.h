#pragma once
#include "PropPageGraphs.h"

class MemGraphs : public CPropPageGraphs
{
public:
	DECLARE_DYNCREATE(MemGraphs)

	MemGraphs();
	~MemGraphs();
	void CreateGraph(int xscale, COLORREF color, DWORD total_mem);
	virtual void CreateSplitter();
	virtual void UpdateGraphs(unsigned int x, const DEVICE *device);
	virtual CGraphWnd2* GetGraphWnd();


	// Dialog Data
	//{{AFX_DATA(CpuGraphs)
	enum { IDD = IDD_MEM_GRAPH };
	//}}AFX_DATA
	
protected:
	// Overrides
	virtual int GetLegendTitles(STRING *title, DEVICE *device, PLOT_ITEM* plot_item);
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CpuGraphs)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation

	// Generated message map functions
	//{{AFX_MSG(CpuGraphs)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	DWORD m_total_mem;
public:
};

