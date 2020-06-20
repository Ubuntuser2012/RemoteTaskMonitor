

#include "stdafx.h"
#include "../ds/ds.h"
extern "C" {
#include "comm.h"
}
#include "resource.h"
#include "DlgThreadPriority.h"
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
#include "dlgWatchCfg.h"
#include "DlgWatchCPU.h"
#include "watchlist.h"

CDlgThreadPriority::CDlgThreadPriority(DEVICE* device, const LIST &ids, CWnd* pParent /*=NULL*/)
: CDialog(CDlgThreadPriority::IDD, pParent)
, m_priority(0)
, m_device(device)
, m_ids(ids)
{

}

void CDlgThreadPriority::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CDlgThreadPriority)
   DDX_Control(pDX, IDC_EDIT_THREAD_PRIORITY, m_edit_prirority);
   //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgThreadPriority, CDialog)
   //{{AFX_MSG_MAP(CDlgThreadPriority)
   ON_WM_CLOSE()
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDlgThreadPriority::OnOK()
{
   char_t data[128];
   m_edit_prirority.UpdateData(TRUE);
   m_edit_prirority.GetWindowText(data, sizeof(data)/sizeof(char_t));
   m_priority = atoiW(data);

   if(m_priority>0 && m_priority<256)
   {
      ITERATOR it;
      int i;
      unsigned int tid;
      for(i=0, tid=(unsigned int )m_ids.GetFirst(it); i<m_ids.GetCount(); tid=(unsigned int )m_ids.GetNext(it), i++)
      {
         m_device->sendSetThreadPriority(tid, m_priority);
      }
   }
   CDialog::OnOK();
}