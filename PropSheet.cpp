// PropSheet.cpp : implementation file
//

#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
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
#include "dlgRemotecpu.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropSheet

IMPLEMENT_DYNAMIC(CPropSheet, CPropertySheet)

CPropSheet::CPropSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
	, m_enable_selchanging_check(true)
{
}

CPropSheet::CPropSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
	, m_enable_selchanging_check(true)
{
}

CPropSheet::~CPropSheet()
{
}


BEGIN_MESSAGE_MAP(CPropSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CPropSheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropSheet message handlers


BOOL CPropSheet::OnInitDialog() 
{
   m_bModeless = FALSE;   
   m_nFlags |= WF_CONTINUEMODAL;

   BOOL bResult = CPropertySheet::OnInitDialog();

   m_bModeless = TRUE;
   m_nFlags &= ~WF_CONTINUEMODAL;
   return bResult;
}

BOOL CPropSheet::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	CDlgRemoteCPU * dlg = (CDlgRemoteCPU*) GetParent();
   NMHDR* pnmh = (NMHDR*)lParam;
   // tab is about to change
   if (TCN_SELCHANGING == pnmh->code && m_enable_selchanging_check)
   {
	   if ((dlg->GetLicenseType() <= LicenseType_Trial && dlg->TrialExpired()) || !dlg->IsLicenseValid() ||
		   dlg->GetNumDiscoveredDevices()>1)
	   {
		  *pResult = 1;
		  return 1;
	   }
	   else
	   {
		  *pResult = 0;
	   }
   }
   // tab has been changed
   else if (TCN_SELCHANGE == pnmh->code)
   {
	   int i = GetActiveIndex ();
	   dlg->OnChangedTab(i);
   }
   return CPropertySheet::OnNotify(wParam, lParam, pResult);
}

void CPropSheet::UnlockTabs()
{
	m_enable_selchanging_check = false;
}

void CPropSheet::LockTabs()
{
	m_enable_selchanging_check = true;
}

void CPropSheet::SetActiveTabAndLock(int nPage)
{
	UnlockTabs();
	GetTabControl()->SetCurFocus(nPage);
	LockTabs();
}

void CPropSheet::SetActivePage(int nPage)
{
   GetTabControl()->SetCurFocus(nPage);
}