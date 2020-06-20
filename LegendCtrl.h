#pragma once


#include <functional>

class STRING;

typedef struct PLOT_DATA_
{
	DWORD pid;
	DWORD tid;
	int imgid;
}
PLOT_DATA;

// LegendCtrl

class LegendCtrl : public CListCtrl
{
	DECLARE_DYNCREATE(LegendCtrl)

public:
	LegendCtrl();
	virtual ~LegendCtrl();

public:
	void MapLegendItemIconToImageListItem(int itemid, int imgid);
	void SetCallbacks(
		std::function<void(const PLOT_DATA *plot_item)>OnRemoveGraphCb,
		std::function<void(const PLOT_DATA *plot_item, COLORREF color)>OnChangeGraphColorCb);
	void Size();
	int InsertItem(const STRING *title, int num_titles, COLORREF color, PLOT_DATA* plot_item);
	int InsertItem(const STRING &title, COLORREF color, PLOT_DATA* plot_item);
	int AddColumn(const STRING &title);
	void UpdateItem(int graph_id, const STRING *values, int num_values);

protected:
	// Generated message map functions
	//{{AFX_MSG(LegendCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnGraphlegendmenuRemovegraph();
	afx_msg void OnGraphlegendmenuChangecolor();
	afx_msg void OnLvnKeydown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	std::function<void(const PLOT_DATA *plot_item)>OnRemoveGraph;
	std::function<void(const PLOT_DATA *plot_item, COLORREF color)>OnChangeGraphColor;
	int GetImageListItem(COLORREF &color);
	void RemoveRows(bool remove_unselected_rows);

	CImageList m_ImageList;
	HMENU m_menu_ps;
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};


