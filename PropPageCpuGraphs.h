#pragma once
#include "PropPageGraphs.h"

#include "../DS/ds.h"

class PropPageCpuGraphs : public PropPageGraphs
{
	struct PROP_PAGE_GRAPH_DATA : public OBJECT
	{
		DWORD m_pid;
		DWORD m_tid;
		STRING m_name;
		COLORREF m_color;

		PROP_PAGE_GRAPH_DATA(DWORD pid, DWORD tid, const char_t* name, COLORREF color)
			: OBJECT()
			, m_pid(pid)
			, m_tid(tid)
			, m_color(color)
		{
			m_name.set(name);
		}
	};

public:
	DECLARE_DYNCREATE(PropPageCpuGraphs)

	PropPageCpuGraphs();
	~PropPageCpuGraphs();
	virtual void CreateSplitter(std::function<void(const PLOT_DATA *plot_item)>OnToggleGenReport);
	virtual void UpdateGraphs(unsigned int x, const DEVICE *device);
	virtual GraphWnd2* GetGraphWnd();
	void CreateGraph(int xscale, COLORREF color, int count);
	BOOL AddGraph(DEVICE *device, PLOT_DATA *plot_item);
	virtual void Size();
	bool HasGraph(const STRING &name, int &idx);
	bool HasGraph(DWORD pid, int &idx);

	// Dialog Data
	//{{AFX_DATA(CpuGraphs)
	enum { IDD = IDD_CPU_GRAPHS_PROP_PAGE };
	//}}AFX_DATA
	
protected:
	// Overrides
	virtual int GetLegendTitles(STRING *title, DEVICE *device, PLOT_DATA* plot_item);
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CpuGraphs)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
	void AddLegendTitles();
	void UpdateGraphColor(const PLOT_DATA *plot_item, COLORREF color);
	COLORREF RecallGraphColor(const DEVICE *device, const PLOT_DATA *plot_item, COLORREF color);

	// Generated message map functions
	//{{AFX_MSG(CpuGraphs)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LIST m_graph_data;
};

