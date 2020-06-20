// DlgWatchCPU.cpp : implementation file
//

#include "stdafx.h"
#include "../ds/ds.h"
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
#include "dlgWatchCfg.h"
#include "dlgWatchBase.h"
#include "dlgWatchCPU.h"
#include "watchlist.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPU dialog


CDlgWatchCPU::CDlgWatchCPU(const LIST &ids, DEVICE* device, const STRING &email_to, CWnd* pParent /*=NULL*/)
: CDlgWatchBase(CDlgWatchCPU::IDD, device, email_to, pParent)
   , m_ids(ids)
{
	//{{AFX_DATA_INIT(CDlgWatchCPU)
	//}}AFX_DATA_INIT
}


void CDlgWatchCPU::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWatchCPU)
	DDX_Control(pDX, IDC_EDIT_CPU_LIMIT, m_edit_cpu);
	DDX_Control(pDX, IDC_EDIT_TIME_LIMIT, m_edit_time);
	DDX_Control(pDX, IDC_EDIT_EMAIL_TO, m_edit_emailto);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWatchCPU, CDialog)
	//{{AFX_MSG_MAP(CDlgWatchCPU)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCPU message handlers

BOOL CDlgWatchCPU::OnInitDialog()
{
   BOOL ret = CDlgWatchBase::OnInitDialog();
   if(ret)
   {
      CListBox *list = (CListBox*)GetDlgItem(IDC_WATCH_LIST);
      if(list)
      {
         int i;
         EXECOBJ *execobj = 0;
         ITERATOR it;
         for(i=0, execobj=(EXECOBJ *)m_ids.GetFirst(it); i<m_ids.GetCount(); execobj=(EXECOBJ *)m_ids.GetNext(it), i++)
         {
            if(execobj)
            {
               PROCESS *p = 0;
               THREAD *t = 0;
               if(execobj->getType()==EXECOBJ_PROCESS)
                  p = dynamic_cast<PROCESS*>(execobj);
               else if(execobj->getType()==EXECOBJ_THREAD)
                  t = dynamic_cast<THREAD*>(execobj);

               STRING msg;
               if(p)
               {
                  msg.sprintf(L"[pid:%i] %s"
                     , p->getId(), p->getName() ? p->getName() : L"");
               }
               else if(t)
               {
                  p = t->getProcess();
                  msg.sprintf(L"[pid:%i] %s [tid:%i] %s"
                     , p->getId(), p->getName() ? p->getName() : L""
                     , t->getId(), t->getName() ? t->getName() : L"");
               }
               int i = list->AddString(msg.s());
               if(i >= 0)
               {
                  list->SetItemData(i, (DWORD)execobj->refptr());
               }
            }
         }
         if(m_ids.GetCount() == 1)
         {
            execobj=(EXECOBJ *)m_ids.GetFirst(it);
            WATCH_ITEM *watch_item = execobj->getCPUWatch();
            if(watch_item)
            {
               STRING s;
               s.sprintf(L"%.02f", watch_item->m_max_cpu);
               GetDlgItem(IDC_EDIT_CPU_LIMIT)->SetWindowText(s.s());
               s.sprintf(L"%i", watch_item->m_duration);
               GetDlgItem(IDC_EDIT_TIME_LIMIT)->SetWindowText(s.s());
            }
         }
      }
   }
   return ret;
}

void CDlgWatchCPU::OnOK() 
{
   CButton *btn;
   char_t *endptr = 0 ;
	CString tmp;

   if(EmailSetupOk())
   {
      GetDlgItem(IDC_EDIT_CPU_LIMIT)->GetWindowText(tmp);
	   float max_cpu = (float)wcstod((LPCTSTR)tmp, &endptr);

	   GetDlgItem(IDC_EDIT_TIME_LIMIT)->GetWindowText(tmp);
	   int duration = _wtoi((LPCTSTR)tmp);

	   btn = (CButton *)GetDlgItem(IDC_CHECK_RESET_DEVICE);
	   bool reset_device = (btn->GetCheck() == BST_CHECKED);

      btn = (CButton *)GetDlgItem(IDC_KILL_PS);
      bool kill_ps = (btn->GetCheck() == BST_CHECKED);

      btn = (CButton *)GetDlgItem(IDC_RESTART_PS);
      bool restart_ps = (btn->GetCheck() == BST_CHECKED);

      int i;
      EXECOBJ *execobj = 0;
      ITERATOR it;
      for(i=0, execobj=(EXECOBJ *)m_ids.GetFirst(it); i<m_ids.GetCount(); execobj=(EXECOBJ *)m_ids.GetNext(it), i++)
      {
         if(execobj)
         {
          PROCESS *p = 0;
          THREAD *t = 0;
          if(execobj->getType()==EXECOBJ_PROCESS)
             p = dynamic_cast<PROCESS*>(execobj);
          else if(execobj->getType()==EXECOBJ_THREAD)
             t = dynamic_cast<THREAD*>(execobj);

          StartCPUWatch(execobj, m_device, max_cpu, p ? p->getId() : 0, t ? t->getId() : 0, duration, m_email_to.s(), reset_device, kill_ps, restart_ps);
         }
         CDlgWatchBase::OnOK(); 
      }
   }
}

void CDlgWatchCPU::StartCPUWatch(EXECOBJ *execobj, DEVICE *device, float max_cpu, DWORD pid, DWORD tid, int duration, const char_t *email_to, bool reset_device, bool kill_ps, bool restart_ps)
{
   if(execobj)
   {
      WATCH_ITEM *watch_item = execobj->getCPUWatch();
      if(!watch_item)
      {
         watch_item = WATCH_ITEM::create(max_cpu, device, pid, tid, duration, email_to, reset_device, kill_ps, restart_ps);
         execobj->setCPUWatch(watch_item);
      }
      else
      {
         watch_item->set(max_cpu, device, pid, tid, duration, email_to, reset_device, kill_ps, restart_ps);
      }
   }
}

void CDlgWatchCPU::OnCancel() 
{
   //m_device->delCPUWatch();
	CDialog::OnCancel();
}

void CDlgWatchCPU::OnDestroy() 
{
      CListBox *list = (CListBox*)GetDlgItem(IDC_WATCH_LIST);
      if(list)
      {
		  for(int i=0; i<list->GetCount(); i++)
		  {
			  EXECOBJ *execobj =  (EXECOBJ *)list->GetItemData(i);
			  if(execobj)
				  execobj->deref();
		  }
	  }
	CDialog::OnDestroy();
}

