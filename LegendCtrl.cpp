// LegendCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "../ds/ds.h"
#include "resource.h"

#include "LegendCtrl.h"

typedef struct LVITEMX_
{
	LVITEM item;
	char_t text[260];
}
LVITEMX;

// LegendCtrl

IMPLEMENT_DYNCREATE(LegendCtrl, CListCtrl)

LegendCtrl::LegendCtrl()
{

}

LegendCtrl::~LegendCtrl()
{
	DestroyMenu(m_menu_ps);
}

BEGIN_MESSAGE_MAP(LegendCtrl, CListCtrl)
	//{{AFX_MSG_MAP(LegendCtrl)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, &LegendCtrl::OnLvnKeydown)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &LegendCtrl::OnNMDblclk)
	ON_NOTIFY_REFLECT(NM_RCLICK, &LegendCtrl::OnNMRClick)
	ON_COMMAND(ID_GRAPHLEGENDMENU_REMOVEGRAPH, OnGraphlegendmenuRemovegraph)
	ON_COMMAND(ID_GRAPHLEGENDMENU_CHANGECOLOR, OnGraphlegendmenuChangecolor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// LegendCtrl message handlers

BOOL LegendCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszClass = WC_LISTVIEW;
	cs.style = LVS_SHOWSELALWAYS | LVS_REPORT | WS_CHILD | WS_VISIBLE;

	return CListCtrl::PreCreateWindow(cs);
}

int LegendCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_ImageList.Create(16, 16, ILC_COLOR24, 1, 64);
	//m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_LIST_ICON));
	SetImageList(&m_ImageList, LVSIL_SMALL);
	SetExtendedStyle(
		LVS_EX_FULLROWSELECT |
		LVS_EX_GRIDLINES |
		//LVS_EX_CHECKBOXES |
		LVS_EX_HEADERDRAGDROP |
		LVS_EX_SUBITEMIMAGES |
		LVS_EX_FLATSB |
		LVS_EX_INFOTIP);

	HINSTANCE hInstance = GetModuleHandle(0);
	m_menu_ps = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_PROCESS));

	return 0;
}

void LegendCtrl::UpdateItem(int graph_id, const STRING *values, int num_values)
{
	LVFINDINFO info;
	memset(&info, 0, sizeof(info));
	info.flags = LVFI_PARAM;
	info.lParam = graph_id;
	int i = FindItem(&info);
	if (i > -1)
	{
		for (int j = 0; j < num_values; j++)
		{
			if (values[j].size())
			{
				SetItemText(i, j, values[j].s());
			}
		}
	}
}

int LegendCtrl::AddColumn(const STRING &title)
{
	LVCOLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.mask |= LVCF_IMAGE;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 120;
	lvColumn.iImage = -1;
	lvColumn.pszText = (LPWSTR)title.s();
	int nCol = GetHeaderCtrl()->GetItemCount();
	nCol = InsertColumn(nCol, &lvColumn);
	return nCol;
}

int LegendCtrl::GetImageListItem(COLORREF &color)
{
	CBitmap bmp, bmp2;
	int imgid;
	char * data[1024];
	BITMAP bmp_s;
	int r = GetRValue(color);
	int g = GetGValue(color);
	int b = GetBValue(color);

	bmp_s.bmType = 0;
	bmp_s.bmWidth = 16;
	bmp_s.bmHeight = 16;
	bmp_s.bmWidthBytes = 64;
	bmp_s.bmPlanes = 1;
	bmp_s.bmBitsPixel = 32;
	bmp_s.bmBits = data;

	bmp.LoadBitmap(IDB_LISTICON);

	bmp.GetBitmapBits(sizeof(data), (void*)data);
	DWORD *pix = (DWORD *)data;
	for (int i = 0; i<sizeof(data) / 4; i++)
	{
		pix[i] = (r << 16) | (g << 8) | (b);
	}
	bmp2.CreateBitmapIndirect(&bmp_s);
	imgid = m_ImageList.Add(&bmp2, RGB(0xff, 0xff, 0xff));
	bmp.DeleteObject();
	bmp2.DeleteObject();

	return imgid;
}

void LegendCtrl::MapLegendItemIconToImageListItem(int itemid, int imgid)
{
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = itemid;
	lvi.iSubItem = 0;
	lvi.iImage = imgid;

	SetItem(&lvi);
}

int LegendCtrl::InsertItem(const STRING &title, COLORREF color, PLOT_DATA* plot_item)
{
	STRING titles[1];
	titles[0] = title;
	return InsertItem(titles, 1, color, plot_item);
}

int LegendCtrl::InsertItem(const STRING *title, int num_titles, COLORREF color, PLOT_DATA* plot_item)
{
	int i = GetItemCount();
	LVITEM item;
	memset(&item, 0, sizeof(LVITEM));
	item.mask = (LVIF_IMAGE | LVIF_STATE | LVIF_TEXT | LVIF_PARAM);
	item.stateMask = LVIS_SELECTED;
	item.iItem = i;
	item.iSubItem = 0;
	item.iImage = GetImageListItem(color);
	item.pszText = (LPWSTR)title[0].s();
	item.cchTextMax = title[0].size()+1;
	if (plot_item)
	{
		plot_item->imgid = item.iImage;
	}
	item.lParam = (int)plot_item;
	int ret = CListCtrl::InsertItem(&item);
	if (ret > -1)
	{
		for (int c = 1; c < num_titles; ++c)
		{
			SetItemText(i, c, (LPWSTR)title[c].s());
		}
	}
	return ret;
}

void LegendCtrl::OnGraphlegendmenuRemovegraph()
{
	RemoveRows(true);
}

void LegendCtrl::RemoveRows(bool remove_unselected_rows)
{
	int i, new_count = 0, old_count;
	LVITEMX _item, *item = 0;
	PLOT_DATA * plot_item = 0;

	old_count = GetItemCount();

	item = new LVITEMX[old_count];
	if (!item)
	{
		goto Exit;
	}
	for (i = 0; i<old_count; i++)
	{
		memset(&_item, 0, sizeof(LVITEMX));
		_item.item.mask = (LVIF_IMAGE | LVIF_STATE | LVIF_TEXT | LVIF_PARAM);
		_item.item.stateMask = LVIS_SELECTED;
		_item.item.iItem = i;
		_item.item.pszText = _item.text;
		_item.item.cchTextMax = sizeof(_item.text) / sizeof(char_t);
		if (GetItem(&_item.item) &&
			*_item.item.pszText &&
			(!remove_unselected_rows || ((_item.item.state&LVIS_SELECTED) != LVIS_SELECTED))
			)
		{
			memcpy(&item[new_count], &_item, sizeof(LVITEMX));
			item[new_count].item.pszText = item[new_count].text;
			new_count++;
		}
		else
		{
			if (strcmpW(_item.item.pszText, TXT("Total")) == 0)
			{
				goto Exit;
			}
			plot_item = (PLOT_DATA *)_item.item.lParam;
			if (plot_item)
			{
				OnRemoveGraph(plot_item);
				//m_ImageList.Remove(plot_item->imgid);
				delete plot_item;
			}
		}
	}
	DeleteAllItems();
	for (i = 0; i<new_count; i++)
	{
		item[i].item.iItem = i;
		CListCtrl::InsertItem(&item[i].item);
	}
	if (GetItemCount() == 0)
	{
		while (m_ImageList.GetImageCount())
		{
			m_ImageList.Remove(0);
		}
	}
Exit:
	if (item)
	{
		delete[] item;
	}
}
void LegendCtrl::OnGraphlegendmenuChangecolor()
{
	int i;
	PLOT_DATA * plot_item = 0;
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK)
	{
		COLORREF color = dlg.GetColor();
		int color_id = GetImageListItem(color);
		POSITION pos = GetFirstSelectedItemPosition();
		while (pos)
		{
			i = GetNextSelectedItem(pos);
			plot_item = (PLOT_DATA *)GetItemData(i);
			if (plot_item)
			{
				plot_item->imgid = color_id;
				OnChangeGraphColor(plot_item, color);
			}
			MapLegendItemIconToImageListItem(i, color_id);
		}
	}
}



void LegendCtrl::OnLvnKeydown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here

	int i = 0;
	if (pLVKeyDow && pLVKeyDow->wVKey)
	{
		switch (pLVKeyDow->wVKey)
		{
		case 'a':
		case 'A':
		{
			if (!GetAsyncKeyState(VK_CONTROL))
				break;
			for (i = 0; i<GetItemCount(); i++)
			{
				SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
			}
			break;
		}
		case 'c':
		case 'C':
			if (!GetAsyncKeyState(VK_CONTROL))
				break;
		case VK_RETURN:
			OnGraphlegendmenuChangecolor();
			break;
		case VK_DELETE:
			OnGraphlegendmenuRemovegraph();
			break;
		}
	}

	*pResult = 0;
}


void LegendCtrl::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	OnGraphlegendmenuChangecolor();
	*pResult = 0;
}


void LegendCtrl::OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	DWORD pos = GetMessagePos();
	CPoint pt(LOWORD(pos), HIWORD(pos));

	TrackPopupMenuEx(GetSubMenu(m_menu_ps, 1), TPM_LEFTALIGN, pt.x, pt.y, m_hWnd, NULL);
	*pResult = 0;
}

void LegendCtrl::SetCallbacks(
	std::function<void(const PLOT_DATA *plot_item)>OnRemoveGraphCb,
	std::function<void(const PLOT_DATA *plot_item, COLORREF color)>OnChangeGraphColorCb)
{
	OnRemoveGraph = OnRemoveGraphCb;
	OnChangeGraphColor = OnChangeGraphColorCb;
}

void LegendCtrl::Size()
{
	RECT rect;
	GetClientRect(&rect);

	SetWindowPos(NULL, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
	UpdateWindow();
}
