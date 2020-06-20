// ListCtrl2.cpp : implementation file
//
//http://www.blueming.de/Help/clistctrl.html

#include "stdafx.h"
#include "../common/types.h"
extern "C" {
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "devicelist.h"
#include "remotecpu.h"
#include "listctrl2.h"
#include "dlgRemotecpu.h"
#include "dlgThreadPriority.h"
#include "dlgStartProcess.h"
#include "dlgRemoteCPU.h"
#include "DlgWatchCfg.h"
#include "DlgWatchMem.h"
#include "DlgWatchCPU.h"
#include "DlgWatchCPUTotal.h"
#include "DlgSelectCore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListCtrl2


//keep this member in sync with enum LIST_COLUMN_ID
const LIST_COLUMN CListCtrl2::m_col[LIST_COLUMN_MAX]={
	{LIST_COLUMN_PROCESS,	LVCFMT_LEFT, 120, -1, L"Process"},
	{LIST_COLUMN_PID,		LVCFMT_RIGHT, 40, -1, L"ID"},
	{LIST_COLUMN_CPU,		LVCFMT_RIGHT, 80, -1, L"CPU" },
	{LIST_COLUMN_CORE,		LVCFMT_RIGHT, 40, -1, L"Core" },
	{LIST_COLUMN_PRIORITY,	LVCFMT_RIGHT, 50, -1, L"Priority"},
	{LIST_COLUMN_CPU_AVG,	LVCFMT_RIGHT, 70, -1, L"Avg CPU"},
	{LIST_COLUMN_CPU_MAX,	LVCFMT_RIGHT, 70, -1, L"Max CPU"},
	{LIST_COLUMN_MEMORY,		LVCFMT_RIGHT, 90, -1, L"Memory"},
	{LIST_COLUMN_MEMORY_MAX, LVCFMT_RIGHT, 80, -1, L"Max Memory"}
};

enum LIST_ICON_INDEX
{
	LIST_ICON_INDEX_PS=0,
	LIST_ICON_INDEX_TH,
	LIST_ICON_INDEX_REPORT,
	LIST_ICON_INDEX_WATCH_CPU,
	LIST_ICON_INDEX_WATCH_CRASH,
};

CListCtrl2::CListCtrl2()
: m_input_handler(0)
, m_parent(0)
, m_erase_bg(true)
, m_pid_in_hex(false)
{
}

void CListCtrl2::SetMenu()
{
	HINSTANCE hInstance = GetModuleHandle(0);
	m_menu_ps = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_PROCESS));
}

void CListCtrl2::SetFont()
{
	LOGFONT lf;
	CFont* f = GetFont( );
	f->GetLogFont(&lf);

	// request a face name "Arial"
#if _MSC_VER==1400
	wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Arial");  
#else #if _MSC_VER==1200
	wcscpy(lf.lfFaceName, L"Arial");  
#endif
	lf.lfWeight *=2;

	m_fontBold.CreateFontIndirect(&lf);  // create the font
}

CListCtrl2::~CListCtrl2()
{
	m_fontBold.DeleteObject(); 
	DestroyMenu(m_menu_ps);
}
//ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
//ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
//ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
//ON_NOTIFY_REFLECT(LVN_ODCACHEHINT, OnOdCacheHint)
//ON_NOTIFY_REFLECT(LVN_ODFINDITEM, OnOdFindItem)


BEGIN_MESSAGE_MAP(CListCtrl2, CListCtrl)
	//{{AFX_MSG_MAP(CListCtrl2)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT (NM_CUSTOMDRAW, OnCustomDraw)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRClick)
	ON_NOTIFY_REFLECT(NM_KEYDOWN, OnKeydown)
	ON_WM_MENUSELECT()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_PROCESS_KILL, OnMenuItemProcessKill)
	ON_COMMAND(ID_PROCESS_START, OnMenuItemProcessStart)
	ON_COMMAND(ID_PROCESS_CLONE, OnMenuItemProcessClone)
	ON_COMMAND(ID_PROCESS_RESTART, OnMenuItemProcessRestart)
	ON_COMMAND(ID_THREAD_KILL, OnMenuItemThreadKill)
	ON_COMMAND(ID_THREAD_SUSPEND, OnMenuItemThreadSuspend)
	ON_COMMAND(ID_THREAD_RESUME, OnMenuItemThreadResume)
	ON_COMMAND(ID_THREAD_PRIORITY, OnMenuItemThreadPriority)
	ON_COMMAND(ID_CPU_WATCH, OnMenuItemCPUWatch)   
	ON_COMMAND(ID_TOTAL_CPU_WATCH, OnMenuItemTotalCPUWatch)   
	ON_COMMAND(ID_PLOT_CPU, OnMenuItemPlotCpuGraph)
	ON_COMMAND(ID_PLOT_MEM, OnMenuItemPlotMemGraph)
	ON_COMMAND(ID_CRASH_REPORT, OnMenuItemCrashReport)
	ON_COMMAND(ID_MEMORY_WATCH, OnMemoryWatch)
	ON_COMMAND(ID_SELECT_CORE, OnMenuItemSelectCore)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrl2 message handlers

BOOL CListCtrl2::Create(CDlgRemoteCPU *input_handler, CWnd *parent)
{
	int nCol = 0;

	m_input_handler = input_handler;
	m_parent = parent;

	BOOL ret = CListCtrl::Create(
		WS_CHILD | 
		WS_VISIBLE | 
		WS_BORDER | 
		LVS_REPORT | 
		//LVS_SINGLESEL |
		LVS_OWNERDATA, 
		CRect(10, 10, 320, 280), parent, 0x285);
	if(!ret)
		goto Exit;
	SetExtendedStyle(
		LVS_EX_FULLROWSELECT | 
		LVS_EX_GRIDLINES |
		//LVS_EX_CHECKBOXES |
		LVS_EX_HEADERDRAGDROP|
		LVS_EX_SUBITEMIMAGES |
		LVS_EX_FLATSB|
		LVS_EX_INFOTIP);

	m_ImageList.Create(16, 16, ILC_MASK, 1, 1);
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_PS));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_TH));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_REPORT));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_WATCH_CPU));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_WATCH_CRASH));
	//m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_CHECKBOX_UNCHECKED));

	SetImageList(&m_ImageList, LVSIL_SMALL);

	for(nCol=0; nCol<LIST_COLUMN_MAX; nCol++)
	{
		LVCOLUMN lvColumn;
		lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
		if(m_col[nCol].image!=-1)
			lvColumn.mask |= LVCF_IMAGE;
		lvColumn.fmt = m_col[nCol].fmt;
		lvColumn.cx = m_col[nCol].width;
		lvColumn.iImage = m_col[nCol].image;
		lvColumn.pszText = (LPWSTR)m_col[nCol].name;
		nCol = InsertColumn(nCol, &lvColumn);
	}

	SetFont();
	SetMenu();

Exit:
	return ret;
}

BOOL CListCtrl2::OnEraseBkgnd(CDC *pDC)
{//To prevent flickering
	if(m_erase_bg)
	{
		m_erase_bg = false;
		return CListCtrl::OnEraseBkgnd(pDC);
	}
	else
	{
		return 0;
	}
}

void CListCtrl2::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	static int prev_count = GetItemCount();
	int count = GetItemCount();
	if(count != prev_count)
	{
		m_erase_bg = true;
		prev_count = count;
	}

	//HD_NOTIFY * phdn = (HD_NOTIFY *) pNMHDR;
	// TODO: Add your control notification handler code here
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;

	int iItemIndx= pItem->iItem;
	EXECOBJ* execobj = (EXECOBJ* )m_input_handler->GetDisplayListObject(iItemIndx);
	PROCESS* p;
	THREAD* t;
	if(!execobj)
	{
		int x=0;
		x=1;
		return;
	}
	char_t text[128];
	float fpercent=0;
	unsigned long memused = 0;

	switch(execobj->getType())
	{
	case EXECOBJ_PROCESS:
		p = dynamic_cast<PROCESS*>(execobj);
		t = 0;
		break;
	case EXECOBJ_THREAD:
		t = dynamic_cast<THREAD*>(execobj);
		p = t->getProcess();
		break;
	}

	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		switch(pItem->iSubItem)
		{
		case LIST_COLUMN_PROCESS: //fill in main text
			switch(execobj->getType())
			{
			case EXECOBJ_PROCESS:
				if(p)
				{
					swprintf(text, L"%s", p ? p->getName() : L"-");
					wcscpy(pItem->pszText, text);
				}
				else
					wcscpy(pItem->pszText, L"-");
				break;
			case EXECOBJ_THREAD:
				if(t && p)
				{
					if(wcslen(t->getName()))
						swprintf(text, L" %s:%s",  p->getName(), t->getName());
					else
						swprintf(text, L" %s",  p->getName());
				}
				else
					swprintf(text, L"-");
				wcscpy(pItem->pszText, text);
				break;
			}
			break;
		case LIST_COLUMN_PID:
			switch(execobj->getType())
			{
			case EXECOBJ_PROCESS:
				if(p)
				{
					m_pid_in_hex ? 
						swprintf(text, L"0x%X", p->getId()) :
						swprintf(text, L"%i", p->getId());
					wcscpy(pItem->pszText, text);
				}
				else
					wcscpy(pItem->pszText, L"-");
				break;
			case EXECOBJ_THREAD:
				if(t && p)
				{
					m_pid_in_hex ? 
						swprintf(text, L"0x%X",  t->getId()) :
						swprintf(text, L"%i",  t->getId());
					;
				}
				else
					swprintf(text, L"-");
				wcscpy(pItem->pszText, text);
				break;
			}
			break;
		case LIST_COLUMN_CPU: 
			switch(execobj->getType())
			{
			case EXECOBJ_PROCESS:
				fpercent=p ? p->cpuPercent(0): 0.0f;
				swprintf(text, L"%.02f",  fpercent);
				break;
			case EXECOBJ_THREAD:
				fpercent=t ? t->cpuPercent(0): 0.0f;
				swprintf(text, L"%.02f",  fpercent);
				break;
			}
			wcscpy(pItem->pszText, text);
			break;
		case LIST_COLUMN_CORE:
		{
			char_t core[32];
			switch (execobj->getType())
			{
			case EXECOBJ_PROCESS:
				swprintf(core, TXT("%i"), p ? p->affinity()+1 : -1);
				swprintf(text, TXT("%s"), p && p->affinity() > -1 ? core : TXT(""));
				break;
			case EXECOBJ_THREAD:
				swprintf(core, TXT("%i"), t ? t->affinity()+1 : -1);
				swprintf(text, TXT("%s"), t && t->affinity() > -1 ? core : TXT(""));
				break;
			}
			wcscpy(pItem->pszText, text);
			break;
		}
		case LIST_COLUMN_CPU_AVG:
			switch(execobj->getType())
			{
			case EXECOBJ_PROCESS:
				fpercent=p ? p->cpuPercent(1): 0.0f;
				swprintf(text, L"%.02f",  fpercent);
				break;
			case EXECOBJ_THREAD:
				fpercent=t ? t->cpuPercent(1): 0.0f;
				swprintf(text, L"%.02f",  fpercent);
				break;
			}
			wcscpy(pItem->pszText, text);
			break;
		case LIST_COLUMN_CPU_MAX: 
			switch(execobj->getType())
			{
			case EXECOBJ_PROCESS:
				fpercent=p ? p->getMaxCpu(): 0.0f;
				swprintf(text, L"%.02f",  fpercent);
				break;
			case EXECOBJ_THREAD:
				fpercent=t ? t->getMaxCpu(): 0.0f;
				swprintf(text, L"%.02f",  fpercent);
				break;
			}
			wcscpy(pItem->pszText, text);
			break;
		case LIST_COLUMN_MEMORY: 
			switch(execobj->getType())
			{
			case EXECOBJ_PROCESS:
				wcscpy(pItem->pszText, p ? PROCESS::memoryUsedHumanReadable(p->memoryUsed()).s() : TXT(""));
				break;
			case EXECOBJ_THREAD:
			default:
				wcscpy(pItem->pszText, TXT(""));
				break;
			}
			break;
		case LIST_COLUMN_MEMORY_MAX: 
			switch(execobj->getType())
			{
			case EXECOBJ_PROCESS:
				memused=p ? p->getMaxMem(): 0;
				break;
			case EXECOBJ_THREAD:
				memused=0;
				break;
			}
			wcscpy(pItem->pszText, PROCESS::memoryUsedHumanReadable(memused));
			break;
		case LIST_COLUMN_PRIORITY: 
			if(t)
				swprintf(text, L"%i", t->getPriority());
			else
				swprintf(text, L"");         
			wcscpy(pItem->pszText, text);
			break;
		}
	}
	if (pItem->mask & LVIF_IMAGE) //valid image?
	{
		switch(pItem->iSubItem)
		{
		case LIST_COLUMN_PROCESS: //fill in main text
			pItem->iImage= t ? LIST_ICON_INDEX_TH:LIST_ICON_INDEX_PS;
			break;
		case LIST_COLUMN_PID:
			pItem->iImage= execobj->getGenReport() ? LIST_ICON_INDEX_REPORT : -1;
			break;
		case LIST_COLUMN_CPU:
			pItem->iImage= execobj->getCPUWatch() ? LIST_ICON_INDEX_WATCH_CPU : -1;
			break;
		case LIST_COLUMN_PRIORITY:
			pItem->iImage= execobj->getCrashWatch() ? LIST_ICON_INDEX_WATCH_CRASH : -1;
			break;
		case LIST_COLUMN_CPU_AVG:
			break;
		case LIST_COLUMN_CPU_MAX:
			break;
		case LIST_COLUMN_MEMORY:
			break;
		case LIST_COLUMN_MEMORY_MAX:
			break;
		}
		//pItem->iImage= m_Items[iItemIndx].m_iImageIndex;
	}
	pItem->mask |= LVIF_STATE;
	pItem->stateMask = LVIS_STATEIMAGEMASK;
	pItem->state = INDEXTOSTATEIMAGEMASK(execobj->getChecked() ? 2:1); 

	*pResult = 0;
}


void CListCtrl2::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLVCUSTOMDRAW  *lplvcd = (NMLVCUSTOMDRAW  *)pNMHDR;
	PROCESS* p = 0;
	THREAD* t = 0;
	int iItemIndx= lplvcd->nmcd.dwItemSpec;
	EXECOBJ* execobj = (EXECOBJ* )m_input_handler->GetDisplayListObject(iItemIndx);
	if(execobj)
	{
		p = dynamic_cast<PROCESS*>(execobj);
		t = dynamic_cast<THREAD*>(execobj);
	}

	switch(lplvcd->nmcd.dwDrawStage) 
	{
	case CDDS_PREPAINT:
		// CDDS_PREPAINT is at the beginning of the paint cycle. You
		// implement custom draw by returning the proper value. In this
		// case, we're requesting item-specific notifications.

		// Request prepaint notifications for each item.
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

		/*
		Because we returned CDRF_NOTIFYITEMDRAW in response to
		CDDS_PREPAINT, CDDS_ITEMPREPAINT is sent when the control is
		about to paint an item.
		*/
	case CDDS_ITEMPREPAINT:
		/*
		To change the font, select the desired font into the
		provided HDC. We're changing the font for every third item
		in the control, starting with item zero.
		*/
		if(p)
		{
			if(p->cpuPercent(0)>m_input_handler->GetRedCpu())
			{
				lplvcd->clrText = RGB(255,0,0);
				lplvcd->clrTextBk = RGB(255,128,128);
			}
			else if(p->getGenReport())
			{
				lplvcd->clrText = RGB(131,0,155);
				lplvcd->clrTextBk = RGB(180,220,250);
			}
			else
			{
				lplvcd->clrText = RGB(0,0,0);
				lplvcd->clrTextBk = RGB(180,220,250);
			}
			SelectObject(lplvcd->nmcd.hdc, m_fontBold);
			*pResult = CDRF_NEWFONT;
		}
		else if(t)
		{
			if(t->cpuPercent(0)>m_input_handler->GetRedCpu())
			{
				lplvcd->clrText = RGB(255,0,0);
				lplvcd->clrTextBk = RGB(255,128,128); //orange
			}
			else if(t->getSuspended())
			{
				lplvcd->clrText = RGB(0,0,0);
				lplvcd->clrTextBk = RGB(250,80,160);
			}
			else if(t->getGenReport())
			{
				lplvcd->clrText = RGB(131,0,155);
				lplvcd->clrTextBk = RGB(100,248,150);
			}
			else
			{
				lplvcd->clrText = RGB(0,0,0);
				lplvcd->clrTextBk = RGB(100,248,150);
				//lplvcd->clrTextBk = RGB(255,200,255);
			}
			*pResult = CDRF_DODEFAULT;
		}
		else 
		{
			*pResult = CDRF_DODEFAULT;
			break;
		}
	}
}

void CListCtrl2::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//HD_NOTIFY * phdn = (HD_NOTIFY *) pNMHDR;
	// TODO: Add your control notification handler code here
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;

	switch(pItem->iItem)
	{
	case LIST_COLUMN_PROCESS:
		m_input_handler->SortBy(SORT_BY_NAME);
		break;
	case LIST_COLUMN_PID:
		m_pid_in_hex ^= 1;
		break;
	case LIST_COLUMN_CPU:
		m_input_handler->SortBy(SORT_BY_CPU);
		break;
	case LIST_COLUMN_CORE:
		m_input_handler->SortBy(SORT_BY_CORE);
		break;
	case LIST_COLUMN_PRIORITY:
		break;
	case LIST_COLUMN_CPU_AVG:
		m_input_handler->SortBy(SORT_BY_CPU_AVG);
		break;
	case LIST_COLUMN_CPU_MAX:
		m_input_handler->SortBy(SORT_BY_CPU_MAX);
		break;
	case LIST_COLUMN_MEMORY:
		m_input_handler->SortBy(SORT_BY_MEM);
		break;
	case LIST_COLUMN_MEMORY_MAX:
		m_input_handler->SortBy(SORT_BY_MEM_MAX);
		break;
	}
}

void CListCtrl2::SetColumnTitle(int ncol, char_t *title)
{
	LVCOLUMN lvColumn;
	memset(&lvColumn, 0, sizeof(lvColumn));
	lvColumn.mask = LVCF_TEXT;
	lvColumn.pszText = title;
	SetColumn(ncol, &lvColumn);
}

void CListCtrl2::Size() 
{
   if(!m_input_handler)
      return;

	CWnd *wndShowThreads=0, *wndShowMem=0, *wndMonitor=0, *wndMemBytes=0;
	RECT rectShowThreads, rectShowMem, rectMonitor, rectMemBytes;

	wndShowThreads = m_input_handler->GetFieldWnd(IDC_CHECK_SHOW_THREADS);
	wndShowThreads->GetWindowRect(&rectShowThreads);

	wndShowMem = m_input_handler->GetFieldWnd(IDC_CHECK_SHOW_MEMORY); 
	wndShowMem->GetWindowRect(&rectShowMem);

	wndMonitor = m_parent;//m_input_handler->GetFieldWnd(IDC_STATIC_MONITOR);
	if(wndMonitor)
	{
		wndMonitor->GetWindowRect(&rectMonitor);
		m_parent->ScreenToClient(&rectMonitor);
		SetWindowPos(0, rectMonitor.left+5, rectMonitor.top+5, rectMonitor.right-rectMonitor.left-5, rectMonitor.bottom-rectMonitor.top-25, 0);
	}

	if(wndShowThreads && wndMonitor)
	{
		wndMonitor->GetWindowRect(&rectMonitor);
		m_parent->ScreenToClient(&rectMonitor);
		wndShowThreads->SetWindowPos(0, rectMonitor.left+6, rectMonitor.bottom-rectMonitor.top-15, rectShowThreads.right-rectShowThreads.left, rectShowThreads.bottom-rectShowThreads.top, 0);
	}
	if(wndShowMem && wndMonitor)
	{
		wndMonitor->GetWindowRect(&rectMonitor);
		m_parent->ScreenToClient(&rectMonitor);
		int xoffset = (rectMonitor.right-rectMonitor.left)/2;
		int width = (rectMonitor.right-rectMonitor.left)/4;
		wndShowMem->SetWindowPos(0, rectMonitor.left+6+xoffset, rectMonitor.bottom-rectMonitor.top-15, 
			width, rectShowThreads.bottom-rectShowThreads.top, 0);
	}
	if(wndMemBytes && wndMonitor)
	{
		wndMonitor->GetWindowRect(&rectMonitor);
		m_parent->ScreenToClient(&rectMonitor);
		int xoffset = (rectMonitor.right-rectMonitor.left)*3/4;
		int width = (rectMonitor.right-rectMonitor.left)/4;
		wndMemBytes->SetWindowPos(0, rectMonitor.left+6+xoffset, rectMonitor.bottom-rectMonitor.top-15, 
			width, rectMemBytes.bottom-rectMemBytes.top, 0);
	}
}

int CListCtrl2::MousePosToListIndex()
{
	// Get the current mouse location and convert it to client
	// coordinates.
	DWORD pos = GetMessagePos();
	CPoint pt(LOWORD(pos), HIWORD(pos));
	ScreenToClient(&pt);

	// Get indexes of the first and last visible items in listview
	// control.
	int index = GetTopIndex();
	int last_visible_index = index + GetCountPerPage();
	if (last_visible_index > GetItemCount())
		last_visible_index = GetItemCount();

	// Loop until number visible items has been reached.
	while (index <= last_visible_index)
	{
		// Get the bounding rectangle of an item. If the mouse
		// location is within the bounding rectangle of the item,
		// you know you have found the item that was being clicked.
		CRect r;
		GetItemRect(index, &r, LVIR_BOUNDS);
		if (r.PtInRect(pt))
		{
			UINT flag = LVIS_SELECTED | LVIS_FOCUSED;
			SetItemState(index, flag, flag);
			break;
		}

		// Get the next item in listview control.
		index++;
	}
	return index;
}

void CListCtrl2::OnNMClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LVHITTESTINFO info;
	DWORD pos = GetMessagePos();
	info.pt.x = MAKEPOINTS(pos).x;
	info.pt.y = MAKEPOINTS(pos).y;
	ScreenToClient(&info.pt);
	int index = SubItemHitTest(&info);
	if (index != -1 && info.flags & LVHT_ONITEMSTATEICON)
	{ 
		PROCESS* p = 0;
		THREAD* t = 0;
		int iItemIndx= MousePosToListIndex();
		EXECOBJ* execobj = (EXECOBJ* )m_input_handler->GetDisplayListObject(iItemIndx);
		if(execobj)
		{
			int newState = execobj->toggleChecked();
			SetItemState(iItemIndx, INDEXTOSTATEIMAGEMASK(newState ? 2:1), LVIS_STATEIMAGEMASK);
		}
	}
	*pResult = 0;
}

void CListCtrl2::OnNMRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMITEMACTIVATE *nmia = (NMITEMACTIVATE *)pNMHDR;
	PROCESS* p = 0;
	THREAD* t = 0;
	int iItemIndx= MousePosToListIndex();
	EXECOBJ* execobj = (EXECOBJ* )m_input_handler->GetDisplayListObject(iItemIndx);
	if(execobj)
	{
		p = dynamic_cast<PROCESS*>(execobj);
		t = dynamic_cast<THREAD*>(execobj);
	}

	DWORD pos = GetMessagePos();
	CPoint pt(LOWORD(pos), HIWORD(pos));

	TrackPopupMenuEx(GetSubMenu(m_menu_ps,0), TPM_LEFTALIGN, pt.x, pt.y, m_hWnd, NULL);

	*pResult = 0;
}

void CListCtrl2::OnMenuSelect( UINT nItemID, UINT nFlags, HMENU hSysMenu  )
{ 
	int x=0;
	switch(nItemID)
	{
	case ID_PROCESS_KILL:
		x=1;
		break;
	case ID_PROCESS_RESTART:
		x=1;
		break;
	case ID_PROCESS_PLOT:
		x=1;
		break;
	case ID_THREAD_KILL:
		x=1;
		break;
	case ID_THREAD_SUSPEND:
		x=1;
		break;
	case ID_THREAD_RESUME:
		x=1;
		break;
	case ID_THREAD_PRIORITY:
		x=1;
		break;
	case ID_THREAD_PLOT:
		x=1;
		break;
	}
}

LRESULT CListCtrl2::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	switch(message)
	{
	case WM_KEYDOWN:
		if(!GetAsyncKeyState(VK_CONTROL))
			break;
		switch(wParam)
		{
		case 'a':
		case 'A':
			for(i=0; i<GetItemCount(); i++)
			{
				SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
			}
			break;
		case 'c':
		case 'C':
			HandleMenu(ID_PROCESS_CLONE);
			break;
		case 'e':
		case 'E':
			HandleMenu(ID_PLOT_MEM);
			break;
		case 'g':
		case 'G':
			HandleMenu(ID_PLOT_CPU);
			break;
		case 'H':
		case 'h':
			HandleMenu(ID_MEMORY_WATCH);
			break;
		case 'k':
		case 'K':
			HandleMenu(ID_PROCESS_KILL);
			HandleMenu(ID_THREAD_KILL);
			break;
		case 'm':
		case 'M':
			HandleMenu(ID_THREAD_RESUME);
			break;
		case 'N':
		case 'n':
			HandleMenu(ID_THREAD_SUSPEND);
			break;
		case 'R':
		case 'r':
			HandleMenu(ID_PROCESS_RESTART);
			break;
		case 's':
		case 'S':
			HandleMenu(ID_PROCESS_START);
			break;
		case 't':
		case 'T':
			HandleMenu(ID_THREAD_PRIORITY);
			break;
		case 'W':
		case 'w':
			HandleMenu(ID_CRASH_REPORT);
			break;
		case 'U':
		case 'u':
			HandleMenu(ID_CPU_WATCH);
			break;
		case 'V':
		case 'v':
			HandleMenu(ID_TOTAL_CPU_WATCH);
			break;
		}
		break;
	}
	return CListCtrl::WindowProc(message, wParam, lParam);
}

void CListCtrl2::OnMenuItemProcessKill()
{
	HandleMenu(ID_PROCESS_KILL);
}
void CListCtrl2::OnMenuItemProcessStart()
{
	HandleMenu(ID_PROCESS_START);
}
void CListCtrl2::OnMenuItemProcessClone()
{
	HandleMenu(ID_PROCESS_CLONE);
}
void CListCtrl2::OnMenuItemProcessRestart()
{
	HandleMenu(ID_PROCESS_RESTART);
}
void CListCtrl2::OnMenuItemThreadKill()
{
	HandleMenu(ID_THREAD_KILL);
}
void CListCtrl2::OnMenuItemThreadSuspend()
{
	HandleMenu(ID_THREAD_SUSPEND);
}
void CListCtrl2::OnMenuItemThreadResume()
{
	HandleMenu(ID_THREAD_RESUME);
}
void CListCtrl2::OnMenuItemThreadPriority()
{
	HandleMenu(ID_THREAD_PRIORITY);
}
void CListCtrl2::OnMenuItemCPUWatch()
{
	HandleMenu(ID_CPU_WATCH);
}
void CListCtrl2::OnMenuItemTotalCPUWatch()
{
	HandleMenu(ID_TOTAL_CPU_WATCH);
}
void CListCtrl2::OnMenuItemPlotCpuGraph()
{
	HandleMenu(ID_PLOT_CPU);
}
void CListCtrl2::OnMenuItemPlotMemGraph()
{
	HandleMenu(ID_PLOT_MEM);
}
void CListCtrl2::OnMenuItemCrashReport()
{
	HandleMenu(ID_CRASH_REPORT);
}
void CListCtrl2::OnMemoryWatch() 
{
	HandleMenu(ID_MEMORY_WATCH);
}
void CListCtrl2::OnMenuItemSelectCore()
{
	HandleMenu(ID_SELECT_CORE);
}

void CListCtrl2::HandleMenu(int id)
{
	int key = m_input_handler->GetLicenseType();

	DEVICE* device = m_input_handler->GetDevice(0);
	if(!device)
		return;
	LIST ids;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		int iItemIndx = GetNextSelectedItem(pos);
		EXECOBJ* execobj = (EXECOBJ* )m_input_handler->GetDisplayListObject(iItemIndx);
		PROCESS* p=0;
		THREAD* t=0;
		if(!execobj)
			continue;
		p = dynamic_cast<PROCESS*>(execobj);
		t = dynamic_cast<THREAD*>(execobj);
		switch(id)
		{
		case ID_PROCESS_KILL:
			if(p)
				device->sendKillProcess(p);
			break;
		case ID_PROCESS_START:
			break;
		case ID_PROCESS_CLONE:
			if(p)
				device->sendCloneProcess(p);
			break;
		case ID_PROCESS_RESTART:
			if(p)
				device->sendRestartProcess(p);
			break;
		case ID_THREAD_KILL:
			if(t)
				device->sendKillThread(t);
			else if(p)
			{
				for(int i=0; i<p->numThreads(); i++)
				{
					t = p->getThread(i);
					if(t)
						device->sendKillThread(t);
				}
			}
			break;
		case ID_THREAD_SUSPEND:
			if(t)
				device->sendSuspendThread(t);
			else if(p)
			{
				for(int i=0; i<p->numThreads(); i++)
				{
					t = p->getThread(i);
					if(t)
						device->sendSuspendThread(t);
				}
			}
			break;
		case ID_THREAD_RESUME:
			if(t)
				device->sendResumeThread(t);
			else if(p)
			{
				for(int i=0; i<p->numThreads(); i++)
				{
					t = p->getThread(i);
					if(t)
						device->sendResumeThread(t);
				}
			}
			break;
		case ID_THREAD_PRIORITY:
			if(t)
				ids.Insert((OBJECT*)t->getId());
			break;
		case ID_SELECT_CORE:
			if (t)
				ids.Insert((OBJECT*)t);
			else if (p)
				ids.Insert((OBJECT*)p);
			break;
		case ID_CPU_WATCH:
				if(t)
					ids.Insert((OBJECT*)t);
				else if(p)
					ids.Insert((OBJECT*)p);
			break;
		case ID_PLOT_CPU:
			{
				PLOT_DATA * plot_item = new PLOT_DATA;
				if (plot_item)
				{
					if (p)
					{
						plot_item->pid = p->getId();
						plot_item->tid = 0;
						m_input_handler->AddCpuGraph(plot_item);
						p->setGenReport(true);
					}
					else if (t)
					{
						plot_item->pid = t->getProcess()->getId();
						plot_item->tid = t->getId();
						m_input_handler->AddCpuGraph(plot_item);
						t->setGenReport(true);
					}
					else
					{
						delete plot_item;
					}
				}
			}
			break;
		case ID_PLOT_MEM:
			{
				PLOT_DATA * plot_item = new PLOT_DATA;
				if (plot_item)
				{
					if (p)
					{
						plot_item->pid = p->getId();
						plot_item->tid = 0;
						m_input_handler->AddMemGraph(plot_item);
						p->setGenReport(true);
					}
					else
					{
						delete plot_item;
					}
				}
			}
			break;
		case ID_CRASH_REPORT:
			{
				DebugRemoteProcess(p);
				pos = 0;//only handle first selected process
			}
			break;
		case ID_MEMORY_WATCH:
			{
				StartMemoryWatch();
			}
			break;
		}
	}
	switch(id)
	{
	case ID_PROCESS_START:
		StartRemoteProcess();
		break;
	case ID_THREAD_PRIORITY:
		SetSelectedThreadPriorities(ids);
		break;
	case ID_CPU_WATCH:
			StartCPUWatch(ids);
		break;
	case ID_TOTAL_CPU_WATCH:
			StartCPUWatch();
		break;
	case ID_SELECT_CORE:
		{
			DlgSelectCore dlg(ids, device->numCores());
			dlg.DoModal();
		}
		break;
	}

	ids.Destroy(false);
}

void CListCtrl2::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
}

void CListCtrl2::SetSelectedThreadPriorities(const LIST &ids)
{
	DEVICE* device = m_input_handler->GetDevice(0);
	CDlgThreadPriority dlg(device, ids);
	SetForegroundWindow();
	dlg.DoModal();
}

void CListCtrl2::StartCPUWatch(const LIST &ids)
{
	DEVICE* device = m_input_handler->GetDevice(0);
	STRING emailto;
	m_input_handler->m_options.GetEmailTo(emailto);
	CDlgWatchCPU dlg(ids, device, emailto);
	SetForegroundWindow();
	dlg.DoModal();
}

void CListCtrl2::StartCPUWatch()
{
	DEVICE* device = m_input_handler->GetDevice(0);
	STRING emailto;
	m_input_handler->m_options.GetEmailTo(emailto);
	CDlgWatchCPUTotal dlg(device, emailto);
	SetForegroundWindow();
	dlg.DoModal();
}

void CListCtrl2::StartMemoryWatch()
{
	DEVICE* device = m_input_handler->GetDevice(0);
	STRING emailto;
	m_input_handler->m_options.GetEmailTo(emailto);
	CDlgWatchMem dlg(device, emailto, 0);
	dlg.DoModal();
}

void CListCtrl2::StartRemoteProcess()
{
	DEVICE* device = m_input_handler->GetDevice(0);
	SetForegroundWindow();
	CDlgStartProcess dlg;
	int nRet = dlg.DoModal();
	wchar_t *path = dlg.GetPsPath();
	switch ( nRet )
	{
	case -1: 
		break;
	case IDABORT:
		break;
	case IDOK:
		{
			if(path && wcslen(path)<MAX_RFILE_PATH)
				device->sendStartProcess(path);
		}
		break;
	case IDCANCEL:
		break;
	default:
		break;
	};
}

void CListCtrl2::DebugRemoteProcess(PROCESS *p)
{
	int nRet;
	DEVICE* device = m_input_handler->GetDevice(0);
	DWORD pid = p ? p->getId() : 0;
	wchar_t* pname = p ?  p->getName() : L"";
	CDlgWatchCfg dlg(pid, pname, m_input_handler);
	nRet = dlg.DoModal();
}

void CListCtrl2::EnableMenuItem(UINT id, BOOL enable)
{
	if(!enable)
		::EnableMenuItem(m_menu_ps, id,  MF_BYCOMMAND|MF_GRAYED);
	else
		::EnableMenuItem(m_menu_ps, id,  MF_BYCOMMAND);
}
