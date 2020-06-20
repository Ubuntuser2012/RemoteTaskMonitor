// TabCtrl2.cpp : implementation file
//

#include "stdafx.h"
#include <winsock2.h>
extern "C"{
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "device.h"
#include "devicelist.h"
#include "remotecpu.h"
#include "dlgRemotecpu.h"
#include "tabctrl2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabCtrl2

CTabCtrl2::CTabCtrl2()
: m_wndParent(0)
{
#if _MSC_VER==1400
	swprintf(m_tabPage[TABPAGE_DEVICE].m_name, sizeof(m_tabPage[TABPAGE_DEVICE].m_name), L"%s", L"Device");
	swprintf(m_tabPage[TABPAGE_PROCESSES].m_name, sizeof(m_tabPage[TABPAGE_PROCESSES].m_name), L"%s", L"Processes");
	swprintf(m_tabPage[TABPAGE_GRAPHS].m_name, sizeof(m_tabPage[TABPAGE_GRAPHS].m_name), L"%s", L"Graphs");
	//swprintf(m_tabPage[TABPAGE_FILTERS].m_name, sizeof(m_tabPage[TABPAGE_FILTERS].m_name), L"%s", L"Filters");
	//swprintf(m_tabPage[TABPAGE_EVENTS].m_name, sizeof(m_tabPage[TABPAGE_EVENTS].m_name), L"%s", L"Events");
	swprintf(m_tabPage[TABPAGE_ABOUT].m_name, sizeof(m_tabPage[TABPAGE_ABOUT].m_name), L"%s", L"About");
#else #if _MSC_VER==1200
	swprintf(m_tabPage[TABPAGE_DEVICE].m_name, L"%s", L"Device");
	swprintf(m_tabPage[TABPAGE_PROCESSES].m_name, L"%s", L"Processes");
	swprintf(m_tabPage[TABPAGE_GRAPHS].m_name, L"%s", L"Graphs");
	//swprintf(m_tabPage[TABPAGE_FILTERS].m_name, L"%s", L"Filters");
	//swprintf(m_tabPage[TABPAGE_EVENTS].m_name, L"%s", L"Events");
	swprintf(m_tabPage[TABPAGE_ABOUT].m_name, L"%s", L"About");
#endif
}

CTabCtrl2::~CTabCtrl2()
{
}


BEGIN_MESSAGE_MAP(CTabCtrl2, CTabCtrl)
	//{{AFX_MSG_MAP(CTabCtrl2)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelchange)
   ON_NOTIFY_REFLECT(TCN_SELCHANGING, OnSelchanging)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabCtrl2 message handlers

BOOL CTabCtrl2::Create(CDlgRemoteCPU *parent)
{
   int nCol = 0;
   m_wndParent = parent;
   BOOL ret = CTabCtrl::Create(
      WS_CHILD | 
      WS_VISIBLE,
      CRect(10, 10, 320, 280), parent, 0x286);
   if(!ret)
	   goto Exit;
   SetExtendedStyle(TCS_EX_FLATSEPARATORS|WS_EX_STATICEDGE);
  
   for(nCol=0; nCol<TABPAGES_TOTAL; nCol++)
   {
	   InsertItem(nCol, m_tabPage[nCol].m_name, 0);
   }


Exit:
   return ret;
}

void CTabCtrl2::Size()
{
	CWnd *wnd2 = m_wndParent->GetDlgItem(IDC_STATIC_MONITOR);
	if(wnd2)
	{
		RECT rect2;
		wnd2->GetWindowRect(&rect2);
		m_wndParent->ScreenToClient(&rect2);
		SetWindowPos(0, rect2.left, rect2.top, rect2.right-rect2.left, rect2.bottom-rect2.top, 0);
	}
   
}

void CTabCtrl2::ShowPageDevices(bool show)
{
	CDialog * dlg = (CDialog*) m_wndParent;
	int flag = show ? SW_SHOW : SW_HIDE;
	dlg->GetDlgItem(IDC_EDIT_IP)->ShowWindow(flag);
	dlg->GetDlgItem(IDC_EDIT_PORT)->ShowWindow(flag);
	dlg->GetDlgItem(IDC_STATIC_IP)->ShowWindow(flag);
	dlg->GetDlgItem(IDC_STATIC_PORT)->ShowWindow(flag);
	dlg->GetDlgItem(IDC_BUTTON_CONNECT)->ShowWindow(flag);
	dlg->GetDlgItem(IDC_STATIC_CONNECT_MESG)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_SYSTEM_INFO)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_MEMORY_INFO)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_BUTTON_RESET_SERVER)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_BUTTON_RESET_DEVICE)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_EDIT_SNAP_INTERVAL)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_BUTTON_GET_SNAPSHOT)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_PERIODIC_SNAPSHOTS)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_BUTTON_SAVE_SNAPSHOT)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_BUTTON_CAPTURE_START)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_BUTTON_CAPTURE_STOP)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_DEVICE)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_SNAPSHOT)->ShowWindow(flag);
}

void CTabCtrl2::ShowPageProcesses(bool show)
{
	int flag = show ? SW_SHOW : SW_HIDE;
   CDialog * dlg = (CDialog*) m_wndParent;

	m_wndParent->GetDisplayList().ShowWindow(flag);
   dlg->GetDlgItem(IDC_CHECK_SHOW_THREADS)->ShowWindow(flag);   
   dlg->GetDlgItem(IDC_CHECK_SHOW_MEMORY)->ShowWindow(flag);   
}

void CTabCtrl2::ShowPageGraphs(bool show)
{
	int flag = show ? SW_SHOW : SW_HIDE;
   CDialog * dlg = (CDialog*) m_wndParent;

	m_wndParent->GetGraphWnd().ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_GRAPH_KEY1)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_GRAPH_KEY2)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_GRAPH_KEY1_TEXT)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_GRAPH_KEY2_TEXT)->ShowWindow(flag);
}

void CTabCtrl2::ShowPageAbout(bool show)
{
   CDialog * dlg = (CDialog*) m_wndParent;
   int flag = show ? SW_SHOW : SW_HIDE;

   dlg->GetDlgItem(IDC_STATIC_ABOUT)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_LOGO)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_EDIT_URL1)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_VERSION)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_EDIT_KEY)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_BUTTON_KEY)->ShowWindow(flag);
   dlg->GetDlgItem(IDC_STATIC_KEY)->ShowWindow(flag);
}

void CTabCtrl2::ShowPage(int pageNum)
{
	switch(pageNum)
	{
	case TABPAGE_DEVICE:
		ShowPageDevices(true);
		ShowPageProcesses(false);
		ShowPageGraphs(false);
		ShowPageAbout(false);
		break;
	case TABPAGE_PROCESSES:
		ShowPageDevices(false);
		ShowPageProcesses(true);
		ShowPageGraphs(false);
		ShowPageAbout(false);
		break;
	case TABPAGE_GRAPHS:
		ShowPageDevices(false);
		ShowPageProcesses(false);
		ShowPageGraphs(true);
		ShowPageAbout(false);
		break;
	case TABPAGE_FILTERS:
		ShowPageDevices(false);
		ShowPageProcesses(false);
		ShowPageGraphs(false);
		ShowPageAbout(false);
		break;
	case TABPAGE_EVENTS:
		ShowPageDevices(false);
		ShowPageProcesses(false);
		ShowPageGraphs(false);
		ShowPageAbout(false);
		break;
	case TABPAGE_ABOUT:
		ShowPageDevices(false);
		ShowPageProcesses(false);
		ShowPageGraphs(false);
		ShowPageAbout(true);
		break;
	}
}

void CTabCtrl2::OnSelchange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	int pageNum = GetCurSel();
	ShowPage(pageNum);
	*pResult = 0;
}

void CTabCtrl2::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult)
{
   CDlgRemoteCPU * dlg = (CDlgRemoteCPU*) m_wndParent;
   if(dlg->GetLicenseType() > LICENSE_TYPE_NONE)
   {
      *pResult = 0;
   }
   else
   {
      *pResult = 1;
   }
}

void CTabCtrl2::SelectTab(int page)
{
   ShowPage(page);
   TabCtrl_SetCurSel(m_hWnd, page);
}
