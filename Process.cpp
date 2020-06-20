#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
#include "../common/util.h"
}
#include "thread.h"
#include "threadlist.h"
#include "Process.h"
#include "ProcessList.h"
#include "watchlist.h"
#include "device.h"

//////////////////////////////////////////////////

PROCESS::~PROCESS()
{
	close();
}

void PROCESS::close()
{
	int i;
	for(i=0; i<m_threads.GetCount(); i++)
	{
		THREAD *t = (THREAD *)m_threads.GetAt(i);
		t->deref();
	}
	m_threads.Destroy();
}

void PROCESS::Init()
{
   EXECOBJ::Init();
	memset(m_data, 0, sizeof(m_data));
	memset(m_timeStamp, 0, sizeof(m_timeStamp));
	//memset(m_cpuTimeSpent, 0, sizeof(m_cpuTimeSpent));
	memset(m_mem, 0, sizeof(m_mem));
	m_crash_watch = 0;
}

PROCESS::PROCESS(EXECOBJ_DATA *data, DEVICE *device)
: EXECOBJ(EXECOBJ_PROCESS, device)
{
	Init();
	memcpy(&m_data[0], data, sizeof(EXECOBJ_DATA));
}

PROCESS* PROCESS::create(EXECOBJ_DATA *data, DEVICE *device)
{
	PROCESS* o = new PROCESS(data, device);
	if(!o)
		throw 0;
	o->m_ref++;
	return o;
}

THREAD *PROCESS::addThread(EXECOBJ_DATA *data)
{
	THREAD *t = m_threads.getThreadById(data->tid);
	if(!t)
	{
		t = THREAD::create(data, this);
		if(t)
		{
			m_threads.Insert2(t, 0, false);
		}
	}
	if(t)
	{
		t->addData(data);
	}
	return t;
}

void PROCESS::addData(EXECOBJ_DATA* data)
{
	switch(m_index)
	{
	case 0:
		m_index=1;
		memcpy(&m_data[m_index], data, sizeof(EXECOBJ_DATA));
		break;
	case 1:
		m_index=2;
		memcpy(&m_data[m_index], data, sizeof(EXECOBJ_DATA));
		break;
	default:
		memcpy(&m_data[1], &m_data[2], sizeof(EXECOBJ_DATA));
		memcpy(&m_data[2], data, sizeof(EXECOBJ_DATA));
	}
}

#if 1//linux
unsigned long PROCESS::cpuTime(bool avg)
{
   unsigned long cput = 0;

   switch(m_index)
   {
   case 0:
      return 0;
   case 1:
      cput = (m_data[1].cpuTimeSpent  > m_data[0].cpuTimeSpent) ? m_data[1].cpuTimeSpent - m_data[0].cpuTimeSpent : 0;
   default:
      cput = avg ? 
         ((m_data[2].cpuTimeSpent > m_data[0].cpuTimeSpent) ? m_data[2].cpuTimeSpent - m_data[0].cpuTimeSpent : 0) :
         ((m_data[2].cpuTimeSpent > m_data[1].cpuTimeSpent) ? m_data[2].cpuTimeSpent - m_data[1].cpuTimeSpent : 0);
   }
   return cput;
}

unsigned long PROCESS::totalTimeDiff(bool avg)
{
   unsigned long cput = 0;

   switch(m_index)
   {
   case 0:
      return 0;
   case 1:
      cput = m_data[1].ft - m_data[0].ft;
	  //cput = (unsigned long)subFileTime(&m_data[1].ft, &m_data[0].ft);
	  break;
   default:
      cput = avg ? 
         m_data[2].ft - m_data[0].ft :
         m_data[2].ft - m_data[1].ft;
		   //cput = (unsigned long)subFileTime(&m_data[2].ft, &m_data[0].ft) :
		   //cput = (unsigned long)subFileTime(&m_data[2].ft, &m_data[1].ft);
	 break;
   }
   return cput;
}
#endif//linux

int PROCESS::affinity() const
{
	return m_index > 1 ? m_data[2].core : 0;
}

float PROCESS::cpuPercent(bool avg)
{
   float ret = 0;
   if(MY_OS_LINUX == m_device->getSysInfo()->os)
   {
      unsigned long cput = cpuTime(avg);
      unsigned long tt = totalTimeDiff(avg);
      ret = tt > 0 ? (cput * 100 / (float)tt) : 0;
   }
   else
   {
      ret = m_threads.cpuPercent(avg);
   }
   if(ret > m_max_cpu)
      m_max_cpu = ret;
   return ret;}

unsigned long PROCESS::memoryUsed() const
{
	if (m_data[2].mem > m_max_mem)
	{
		m_max_mem = m_data[2].mem;
	}
	return m_data[2].mem;
}

STRING PROCESS::memoryUsedHumanReadable(unsigned long memused)
{
	STRING ret;

	if (memused > 1024 * 1024 * 1024)
		ret.sprintf(TXT("%.02f GB"), memused / 1024.0 / 1024.0 / 1024.0);
	else if (memused > 1024 * 1024)
		ret.sprintf(TXT("%.02f MB"), memused / 1024.0 / 1024.0);
	else if (memused > 1024)
		ret.sprintf(TXT("%.02f KB"), memused / 1024.0);
	else
		ret.sprintf(TXT("%.02f B"), memused);
	return ret;
}

void PROCESS::setGenReport(bool set) 
{
   int i;
   for(i=0; i<m_threads.GetCount(); i++)
   {
      THREAD *thread = (THREAD *)m_threads.GetAt(i);
      thread->setGenReport(set);
   }
   EXECOBJ::setGenReport(set);
}

bool PROCESS::getGenReport() const 
{ 
   return EXECOBJ::getGenReport();
}
void PROCESS::setAffinity(int core) const
{
	m_device->sendSetProcessAffinity(m_data[0].pid, core);
}