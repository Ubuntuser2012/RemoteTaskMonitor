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
#include "dlgWatchMemAlert.h"
#include "dlgWatchCPUAlert.h"
#include "dlgWatchCPUTotalAlert.h"
#include "dlgRemoteCPU.h"

void py_memwatch_alert(STRING &msg);
void py_totalcpuwatch_alert(STRING &msg);
void py_pscpuwatch_alert(int pid, STRING &msg);
void py_thcpuwatch_alert(int pid, int tid, STRING &msg);

int WATCH_LIST::Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const
{
	WATCH_ITEM *d1 = (WATCH_ITEM*) visited_obj;
	WATCH_ITEM *d2 = (WATCH_ITEM*) user_obj;
	
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	else 
		return 0;
}

WATCH_ITEM* WATCH_LIST::getItem(int id)
{
	WATCH_ITEM **item = 0;
	WATCH_ITEM *key = (WATCH_ITEM *)id;
	if(Find(key, (OBJECT***)&item) && *item)
	{
		return *item;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////


WATCH_ITEM* WATCH_ITEM::create(bool debug_new_ps, DWORD pid, const char_t *pspath, const char_t *mapfile, const char_t *emailto,
							   const char_t *pspath2, bool watch_ps2, bool reset_device, bool silent_mode)
{
	WATCH_ITEM* o = new WATCH_ITEM(debug_new_ps, pid, pspath, mapfile, emailto,
		pspath2, watch_ps2, reset_device, silent_mode);
	
	if(!o)
		throw 0;
	o->m_ref++;
	return o;
}

WATCH_ITEM::WATCH_ITEM(
					   bool debug_new_ps, 
                       DWORD pid, 
                       const char_t *pspath, 
                       const char_t *mapfile, 
                       const char_t *emailto,
					   const char_t *pspath2, 
					   bool watch_ps2, 
					   bool reset_device, 
					   bool silent_mode)
					   :m_debug_new_ps(debug_new_ps)
					   ,m_pid(pid)
					   ,m_tid(0)
					   ,m_pspath(pspath)
					   ,m_mapfile(mapfile)
					   ,m_emailto(emailto)
					   ,m_pspath2(pspath2)
					   ,m_watch_ps2(watch_ps2)
					   ,m_reset_device(reset_device)
					   ,m_silent_mode(silent_mode)
					   
					   ,m_min_mem(0)
					   ,m_duration(0)
					   ,m_started_at(0)
					   
					   ,m_max_cpu(0.0f)
					   ,m_kill_ps(0)
					   ,m_restart_ps(0)
					   
					   , m_dlgMemWatchAlert(0)
					   , m_dlgCPUWatchAlert(0)
					   , m_dlgTotalCPUWatchAlert(0)
					   , m_device(0)
{
}

WATCH_ITEM* WATCH_ITEM::create(
							   int min_mem,
							   DEVICE * device,
							   int duration,
							   const char_t *emailto,
							   bool reset_device)
{
	WATCH_ITEM* o = new WATCH_ITEM(min_mem, device, duration, emailto, reset_device);
	
	if(!o)
		throw 0;
	o->m_ref++;
	return o;
}

WATCH_ITEM::WATCH_ITEM(
					   int min_mem,
					   DEVICE * device,
					   int duration,
					   const char_t *emailto,
					   bool reset_device)
					   :m_debug_new_ps(0)
					   ,m_pid(0)
					   ,m_tid(0)
					   ,m_pspath()
					   ,m_mapfile()
					   ,m_emailto(emailto)
					   ,m_pspath2()
					   ,m_watch_ps2()
					   ,m_reset_device(reset_device)
					   ,m_silent_mode(0)
					   
					   ,m_min_mem(min_mem)
					   ,m_duration(duration)
					   ,m_started_at(0)
					   
					   ,m_max_cpu(0.0f)
					   ,m_kill_ps(0)
					   ,m_restart_ps(0)
					   
					   , m_dlgMemWatchAlert(0)
					   , m_dlgCPUWatchAlert(0)
					   , m_dlgTotalCPUWatchAlert(0)
					   , m_device(device)
{
	   m_dlgMemWatchAlert = new CDlgWatchMemAlert(device, 0);
}

void WATCH_ITEM::set(
					 int min_mem,
					 DEVICE * device,
					 int duration,
					 const char_t *emailto,
					 bool reset_device)
{
	m_min_mem = min_mem;
	m_duration = duration;
	if(emailto)
		m_emailto.set(emailto);
	else
		m_emailto.clear();
	m_reset_device=reset_device;
	
	m_device = device;
}

WATCH_ITEM* WATCH_ITEM::create(
							   float max_cpu,
							   DEVICE *device, 
							   DWORD pid,
							   DWORD tid,
							   int duration,
							   const char_t *emailto,
							   bool reset_device,
							   bool kill_ps,
							   bool restart_ps)
{
	WATCH_ITEM* o = new WATCH_ITEM(max_cpu, device, pid, tid, duration, emailto, reset_device, kill_ps, restart_ps);
	
	if(!o)
		throw 0;
	o->m_ref++;
	return o;
}

WATCH_ITEM::WATCH_ITEM(
					   float max_cpu,
					   DEVICE *device, 
					   DWORD pid,
					   DWORD tid,
					   int duration,
					   const char_t *emailto,
					   bool reset_device,
					   bool kill_ps,
					   bool restart_ps)
					   :m_debug_new_ps(0)
					   ,m_pid(pid)
					   ,m_tid(tid)
					   ,m_pspath()
					   ,m_mapfile()
					   ,m_emailto(emailto)
					   ,m_pspath2()
					   ,m_watch_ps2()
					   ,m_reset_device(reset_device)
					   ,m_silent_mode(0)
					   
					   ,m_min_mem(0)
					   ,m_duration(duration)
					   ,m_started_at(0)
					   
					   ,m_max_cpu(max_cpu)
					   ,m_kill_ps(kill_ps)
					   ,m_restart_ps(restart_ps)
					   
					   , m_dlgMemWatchAlert(0)
					   , m_dlgCPUWatchAlert(0)
					   , m_dlgTotalCPUWatchAlert(0)
					   , m_device(device)
{
	   m_dlgCPUWatchAlert = new CDlgWatchCPUAlert(device, this, 0);
}

void WATCH_ITEM::set(
					 float max_cpu,
					 DEVICE *device, 
					 DWORD pid,
					 DWORD tid,
					 int duration,
					 const char_t *emailto,
					 bool reset_device,
					 bool kill_ps,
					 bool restart_ps)
{
	m_max_cpu = max_cpu;
	m_duration = duration;
	if(emailto)
		m_emailto.set(emailto);
	else
		m_emailto.clear();
	m_reset_device=reset_device;
	m_kill_ps = kill_ps;
	m_restart_ps = restart_ps;
	m_device = device;
}

WATCH_ITEM* WATCH_ITEM::create(
							   float max_cpu,
							   DEVICE *device, 
							   int duration,
							   const char_t *emailto,
							   bool reset_device)
{
	WATCH_ITEM* o = new WATCH_ITEM(max_cpu, device, duration, emailto, reset_device);
	
	if(!o)
		throw 0;
	o->m_ref++;
	return o;
}

WATCH_ITEM::WATCH_ITEM(
					   float max_cpu,
					   DEVICE *device, 
					   int duration,
					   const char_t *emailto,
					   bool reset_device)
					   :m_debug_new_ps(0)
					   ,m_pid(0)
					   ,m_tid(0)
					   ,m_pspath()
					   ,m_mapfile()
					   ,m_emailto(emailto)
					   ,m_pspath2()
					   ,m_watch_ps2()
					   ,m_reset_device(reset_device)
					   ,m_silent_mode(0)
					   
					   ,m_min_mem(0)
					   ,m_duration(duration)
					   ,m_started_at(0)
					   
					   ,m_max_cpu(max_cpu)
					   ,m_kill_ps()
					   ,m_restart_ps(0)
					   
					   , m_dlgMemWatchAlert(0)
					   , m_dlgCPUWatchAlert(0)
					   , m_dlgTotalCPUWatchAlert(0)
					   , m_device(device)
{
	m_dlgTotalCPUWatchAlert = new CDlgWatchCPUTotalAlert(device, 0);
}

void WATCH_ITEM::set(
					 float max_cpu,
					 DEVICE *device, 
					 int duration,
					 const char_t *emailto,
					 bool reset_device)
{
	m_max_cpu = max_cpu;
	m_duration = duration;
	if(emailto)
		m_emailto.set(emailto);
	else
		m_emailto.clear();
	m_reset_device=reset_device;
	m_device = device;   
}

WATCH_ITEM::~WATCH_ITEM()
{
	if(m_dlgMemWatchAlert)
		delete m_dlgMemWatchAlert;
	
	if(m_dlgCPUWatchAlert)
		delete m_dlgCPUWatchAlert;
	
}

void WATCH_ITEM::trigger(bool condition, DEVICE *device)
{
	CDialog *dlg = 0;
	int dlg_id = 0;
	bool mem_watch = 0;
	bool cpu_watch = 0;
	bool total_cpu_watch = 0;
	PROCESS *p = 0;
	THREAD *t = 0;
	
	if(m_dlgMemWatchAlert)
	{
		dlg = m_dlgMemWatchAlert;
		dlg_id = IDD_MEMORY_WATCH_ALERT;
		mem_watch = true;
	}
	else if(m_dlgCPUWatchAlert)
	{
		dlg = m_dlgCPUWatchAlert;
		dlg_id = IDD_CPU_WATCH_ALERT;
		cpu_watch = true;
	}
	else if(m_dlgTotalCPUWatchAlert)
	{
		dlg = m_dlgTotalCPUWatchAlert;
		dlg_id = IDD_TOTAL_CPU_WATCH_ALERT;
		total_cpu_watch = true;
	}
	else
	{
		return;
	}
	
	p = m_device->getProcessById(m_pid);
	if(p)
		t = p->getThreadById(m_tid);
	
	if(condition)
	{
		if(0==m_started_at)
		{
			m_started_at = GetTickCount();
		}
		else if(GetTickCount() - m_started_at > m_duration * 1000)
		{
			if(m_reset_device)
			{
				device->sendResetDevice();
			}
			if(cpu_watch && p)
			{
				if(m_restart_ps)
				{
					device->sendRestartProcess(p);
				}
				else if(m_kill_ps)
				{
					device->sendKillProcess(p);
				}
			}
			if(dlg)
			{
				STRING msg, tmp, subject;

				if(!dlg->m_hWnd && !m_silent_mode)
					dlg->Create(dlg_id, 0);
				CEdit *wnd = dlg->m_hWnd ? (CEdit * )dlg->GetDlgItem(IDC_STATIC_ALERT_MSG) : 0;
				
				if(cpu_watch)
				{
					if(p && t)
					{
						msg.sprintf(TXT("[pid:%i]%s [tid:%i]%s"),
							m_pid, 
							p->getName() ? p->getName() : TXT(""),
							m_tid,
							t->getName() ? t->getName() : TXT(""));
					}
					else if(p)
					{
						msg.sprintf(TXT("[pid:%i]%s"),
							m_pid, 
							p->getName() ? p->getName() : TXT(""));
					}
				}
				
				if(mem_watch)
				{
					subject.set(L"Remote Task Monitor - Memory watch alert was triggered");
					tmp.sprintf(L"Free memory < %i kB", m_min_mem/1024);
				}
				else if(cpu_watch || total_cpu_watch)
				{
					subject.set(L"Remote Task Monitor - CPU watch alert was triggered");
					tmp.sprintf(L"CPU > %.02f %%", m_max_cpu);
				}
				
				msg.append(tmp.s());
				
				if(wnd)
					wnd->SetWindowText(msg.s());

				if(dlg->m_hWnd && !dlg->IsWindowVisible())
					dlg->ShowWindow(SW_SHOW);

			   if(m_emailto.size())
			   {
				   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
				   mainWnd->SendEmail(m_emailto, subject, msg);
			   }

            //notify script
            if(mem_watch)
            {
               py_memwatch_alert(msg);
            }
            if(total_cpu_watch)
            {
               py_totalcpuwatch_alert(msg);
            }
            if(cpu_watch)
            {
               if(p && t)
                  py_thcpuwatch_alert(m_pid, m_tid, msg);
               else if(p)
                  py_pscpuwatch_alert(m_pid, msg);
            }
			}
		}
	}
	else if(0!=m_started_at)
	{
		m_started_at = 0;
		//if(dlg->m_hWnd)
		//	dlg->ShowWindow(SW_HIDE);
	}
}


