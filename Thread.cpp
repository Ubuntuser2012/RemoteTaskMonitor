#include "stdafx.h"
#include "../common/types.h"
#include "../common/util.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "Thread.h"
#include "ThreadList.h"
#include "Process.h"
#include "ProcessList.h"
#include "watchlist.h"
#include "device.h"

void THREAD::Init()
{
   EXECOBJ::Init();
   memset(m_data, 0, sizeof(m_data));
   memset(m_name, 0, sizeof(m_name));
   m_suspended = 0;
}

THREAD::THREAD(EXECOBJ_DATA *data, PROCESS *p)
: EXECOBJ(EXECOBJ_THREAD, p->getDevice())
, m_process(p)
{
   Init();
   memcpy(&m_data[0], data, sizeof(EXECOBJ_DATA));
   if(data->name && *data->name)
      setName(data->name);
}

THREAD* THREAD::create(EXECOBJ_DATA *data, PROCESS *p)
{
	THREAD* o = new THREAD(data, p);
	if(!o)
		throw 0;
	o->m_ref++;
	return o;
}

void THREAD::addData(EXECOBJ_DATA* data)
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

long64 THREAD::cpuTime(bool avg)
{
   long64 cput = 0;

   switch(m_index)
   {
   case 0:
      return 0;
   case 1:
      cput = (m_data[1].cpuTimeSpent > m_data[0].cpuTimeSpent) ? m_data[1].cpuTimeSpent - m_data[0].cpuTimeSpent : 0;
   default:
      cput = avg ? 
         ((m_data[2].cpuTimeSpent > m_data[0].cpuTimeSpent) ? m_data[2].cpuTimeSpent - m_data[0].cpuTimeSpent : 0) :
         ((m_data[2].cpuTimeSpent > m_data[1].cpuTimeSpent) ? m_data[2].cpuTimeSpent - m_data[1].cpuTimeSpent : 0);
   }
   return cput;
}

long64 THREAD::totalTimeDiff(bool avg)
{
   long64 cput = 0;

   switch(m_index)
   {
   case 0:
      return 0;
   case 1:
      cput = m_data[1].ft - m_data[0].ft;
	   //cput = subFileTime(&m_data[1].ft, &m_data[0].ft);
	  break;
   default:
      cput = avg ? 
         m_data[2].ft - m_data[0].ft:
         m_data[2].ft - m_data[1].ft;
		   //subFileTime(&m_data[2].ft, &m_data[0].ft) :
		   //subFileTime(&m_data[2].ft, &m_data[1].ft);
		 break;
   }
   return cput;
}

float THREAD::cpuPercent(bool avg)
{
   long64 cput = cpuTime(avg);
   long64 tt = totalTimeDiff(avg);
   float ret = tt > 0 ? (cput * 100 / (float)tt) : 0;
   if(MY_OS_WINCE == m_device->getSysInfo()->os)
   {
      ret /= m_device->getSysInfo()->numberOfProcessors();
   }
   if(ret > m_max_cpu)
      m_max_cpu = ret;
   return ret;

}

int THREAD::getPriority()
{
   return m_data[m_index].priority;
}

int THREAD::affinity() const
{
	return m_index > 1 ? m_data[2].core : 0;
}

void THREAD::setAffinity(int core) const
{
	m_device->sendSetThreadAffinity(m_data[0].tid, core);
}