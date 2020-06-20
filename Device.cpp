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
#include "resource.h"
#include "dlgDbgPsInfo.h"
#include "dlgWatchMemAlert.h"
#include "dlgWatchCPUAlert.h"
#include "dlgRemoteCPU.h"
#include "../common/util.h"

void py_got_pspath(DWORD pid, const char_t *name);
void py_crashwatch_ack(int pid, STRING &path, bool ok);
void py_got_exception(int pid, const STRING &path, const STRING &excmsg);

static float calculatePercentage(long64 tt, long64 cpuidle)
{
	if(tt <= 0 || cpuidle < 0 || cpuidle > tt)
	{
		return 0;
	}
	else
	{
		return (tt - cpuidle) * 100 / (float)tt;
	}
}

unsigned long MY_SYSTEM_INFO::numberOfProcessors()  const
{
   switch(os)
   {
   case MY_OS_WINCE:
      return w.dwNumberOfProcessors;
   case MY_OS_LINUX:
      return l.numberOfProcessors;
   default:
      return 0;
   }
}
void MY_SYSTEM_INFO::processorArchitecture(char_t *architecture, int sz) const
{
   switch(os)
   {
   case MY_OS_WINCE:
	   switch(w.wProcessorArchitecture)
	   {
	   case PROCESSOR_ARCHITECTURE_INTEL://            0
		   swprintf(architecture, TXT("INTEL"));
		   break;
	   case PROCESSOR_ARCHITECTURE_MIPS://             1
		   swprintf(architecture, TXT("MIPS"));
		   break;
	   case PROCESSOR_ARCHITECTURE_ALPHA://            2
		   swprintf(architecture, TXT("ALPHA"));
		   break;
	   case PROCESSOR_ARCHITECTURE_PPC://              3
		   swprintf(architecture, TXT("PPC"));
		   break;
	   case PROCESSOR_ARCHITECTURE_SHX://              4
		   swprintf(architecture, TXT("SHX"));
		   break;
	   case PROCESSOR_ARCHITECTURE_ARM://              5
		   swprintf(architecture, TXT("ARM"));
		   break;
	   case PROCESSOR_ARCHITECTURE_IA64://             6
		   swprintf(architecture, TXT("IA64"));
		   break;
	   case PROCESSOR_ARCHITECTURE_ALPHA64://          7
		   swprintf(architecture, TXT("ALPHA64"));
		   break;
		   //   case PROCESSOR_ARCHITECTURE_MSIL://             8
		   //      swprintf(architecture, TXT("MSIL"));
		   //      break;
		   //   case PROCESSOR_ARCHITECTURE_AMD64://            9
		   //      swprintf(architecture, TXT("AMD64"));
		   //      break;
		   //   case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64://    10
		   //      swprintf(architecture, TXT("IA32_ON_WIN64"));
		   //      break;
	   case PROCESSOR_ARCHITECTURE_UNKNOWN:// 0xFFFF
		   swprintf(architecture, TXT("UNKNOWN"));
		   break;
	   default:
		   swprintf(architecture, TXT(""));
		   break;
	   }
      break;
   case MY_OS_LINUX:
      wcscpy(architecture, l.processorArchitecture);
      break;
   }
}

   
DEVICE::~DEVICE()
{
   close();

   if(m_stream)
      delete [] m_stream;
}

DEVICE::DEVICE(unsigned long ip, unsigned short port, SOCKET so)
: m_streamSize(0)
, m_stream(0)
, m_index(0)
, m_streamHead(0)
, m_streamTail(0)
, m_mem_watch(0)
, m_cpu_watch(0)
{
   m_so = so;
   m_ip = ip;
   m_port = port;
   memset(&m_snapinfo, 0, sizeof(m_snapinfo));
   memset(&m_sysinfo, 0, sizeof(m_sysinfo));
   memset(m_max_cpu, 0, sizeof(m_max_cpu));

   m_init_settings.capture_on_dev = m_init_settings.priority = m_init_settings.snap_interval = -1;

   m_proceses.setDevice(this);
}

DEVICE* DEVICE::create(unsigned long ip, unsigned short port, SOCKET so)
{
   DEVICE* o = new DEVICE(ip, port, so);
   if(!o)
      throw 0;
   o->m_ref++;

   o->m_streamSize = 1024*1024*5;
   o->m_stream = new char [o->m_streamSize];
   if(!o->m_stream)
      throw 0;
   return o;
}

unsigned long DEVICE::getIp() const
{
	return m_ip;
}

unsigned short DEVICE::getPort() const
{
	return m_port;
}

void DEVICE::gotProcessPath(DWORD pid, const char_t *name)
{
   if(name)
   {
      PROCESS* p = m_proceses.getProcessById(pid);
      if(p)
      {
         p->setPath(name);
         py_got_pspath(pid, name);
      }
   }
}

void DEVICE::setThreadName(DWORD tid, char_t name[64])
{
   int i;
   for(i=0; i<m_proceses.GetCount(); i++)
   {
      PROCESS* p = (PROCESS*)m_proceses.GetAt(i);
      if(p)
      {
         THREAD *t = p->getThreadById(tid);
         if(t)
         {
            t->setName(name);
            break;
         }
      }
   }
}

void DEVICE::removeDeadProcesses()
{
	m_proceses.removeUntaggedProcesses();
    m_proceses.resetTag(TAG_TYPE_LIVE_PS);
}

int DEVICE::checkVersion(int server_version, int client_version, STRING &err)
{
	int ret = 1;
	if(server_version!=client_version &&
	   server_version!=client_version-1 &&
	   server_version!=client_version-2 &&
	   server_version!=client_version-3 &&
	   server_version!=client_version-4)
	{
	  err.sprintf(L"Server version %.02f. Client version %.02f", server_version/100.0f, client_version/100.0f);
	  ret = 0;
	}
	return ret;
}

int DEVICE::consumeStreamData()
{
	int consumed_bytes = 0;
	while (m_streamTail > m_streamHead)
	{
		COMM_MESG *msg = (COMM_MESG*)&m_stream[m_streamHead];
		{
			if (msg->sz > m_streamTail - m_streamHead)
			{
				//need more data
				return 0;
			}

			STRING err;
			int parsed_bytes = parseMessage(msg, err);
			if (parsed_bytes > 0)
			{
				m_streamHead += parsed_bytes;
				consumed_bytes += parsed_bytes;
			}
			else
			{
				return 0;
			}
		}
	}

	memmove(&m_stream[0], &m_stream[m_streamHead], m_streamTail - m_streamHead);
	m_streamTail = m_streamTail - m_streamHead;
	m_streamHead = 0;

	return consumed_bytes;
}

int DEVICE::readSocket(STRING &err)
{
   int ret=0;

   while(m_streamSize > m_streamTail)
   {
      ret = socket_read(m_so, (unsigned char*)&m_stream[m_streamTail], m_streamSize-m_streamTail, 0);
	  if(ret>0)
      {
         m_streamTail += ret;
      }
      else if(ret < 0 && ret != -WSAEWOULDBLOCK)
      {
		logg(L"socket_read error!\n");
		break;
      }
      else if(ret == 0 || ret == -WSAEWOULDBLOCK)//nothing was read, socket was closed?
      {
		 break;
      }
   }
   return ret;
}

int DEVICE::parseMessage(COMM_MESG *msg, STRING &err)
{
	int ret = 0;
	switch (msg->cmd)
	{
		case COMM_CMD_PDATA:
		{
			EXECOBJ_DATA *data = (EXECOBJ_DATA *)(&msg[1]);
			int num = msg->numelm;
			for (int i = 0; i<num; i++)
			{
				/*wprintf(L"%i\t%s\t%iMB\n",
				data[i].pid,
				data[i].name,
				data[i].hsz/1024/1024);*/
				m_proceses.addProcess(&data[i], this);//this will tag the process, at the end, any untagged process will be removed, assumed dead
			}
			return msg->sz;
		}
		case COMM_CMD_TDATA:
		{
			EXECOBJ_DATA *data = (EXECOBJ_DATA *)(&msg[1]);
			int num = msg->numelm;
			for (int i = 0; i<num; i++)
			{
				if (data[i].cpuTimeSpent>0)
					wprintf(L"%i:%i\t%llu%\n",
						data[i].pid,
						data[i].tid,
						data[i].cpuTimeSpent);
				m_proceses.addThread(&data[i]);
			}
			return msg->sz;
		}
		case COMM_CMD_SNAPINFO:
		{
			COMM_SNAPINFO *data = (COMM_SNAPINFO *)(&msg[1]);
			int num = msg->numelm;
			for (int i = 0; i<num; i++)
			{
				wprintf(L"Now:%llu\tRAM: %i%%\nRAM %i/%i\n",
					data[i].now,
					memPercent(),
					data[i].memstatus.freeMemKB,
					data[i].memstatus.totalMemKB);
				addSnapInfo(&data[i]);
				removeDeadProcesses();
			}
			return msg->sz;
		}
		case COMM_CMD_INIT_SETTINGS:
		{
			COMM_INIT_SETTINGS *data = (COMM_INIT_SETTINGS *)(&msg[1]);
			memcpy(&m_init_settings, data, sizeof(COMM_INIT_SETTINGS));
			return msg->sz;
		}
		case COMM_CMD_SYSINFO:
		{
			MY_SYSTEM_INFO *data = (MY_SYSTEM_INFO *)(&msg[1]);
			if (!checkVersion(msg->id, RTMON_VERSION, err))
			{
				return -1;
			}
			memcpy(&m_sysinfo, data, sizeof(MY_SYSTEM_INFO));
			return msg->sz;
		}
		case COMM_CMD_DEBUG_NEW_PROCESS:
		case COMM_CMD_DEBUG_ACTIVE_PROCESS:
			debugProcessAckRecvd((DEBUG_PROCESS_ACK *)(msg));
			return msg->sz;
		case COMM_CMD_CRASH_INFO:
			debugProcessCrashInfoRecvd((CRASH_INFO *)(msg));
			return msg->sz;
		case COMM_CMD_TNAME:
		{
			THREAD_NAME *data = (THREAD_NAME *)(&msg[1]);
			int num = msg->numelm;
			for (int i = 0; i<num; i++)
			{
				setThreadName(data[i].tid, data[i].name);
			}
			return msg->sz;
		}
		case COMM_CMD_CLIENTS_MAXED:
		{
			int num = msg->numelm;
			err.sprintf(L"Maximum %i clients already connected", num);
			m_streamHead += msg->sz;
			return -1;
		}
		case COMM_CMD_GET_PROCESS_PATH:
		{
			FILE_DATA *data = (FILE_DATA *)(&msg[1]);
			STRING path;
			path.set(data->data);
			gotProcessPath(data->msg.id, path.s());
			return msg->sz;
		}
		case COMM_CMD_NOOP:
		{
			return msg->sz;
		}
		case COMM_CMD_CLOSE:
		{
		}
		default:
		{
		}
	}
	return 0;
}

int DEVICE::sendNoop()
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_NOOP;
   msg.numelm = 1;
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendGetThreadNames()
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_TNAME;
   msg.numelm = 1;
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendGetMemoryInfo(bool show)
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_GET_MEMINFO;
   msg.show = show;
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);     
   logg(L"COMM_CMD_GET_MEMINFO show: %s\n", show ? L"yes" : L"no");
   return ret;
}

int DEVICE::sendKillProcess(PROCESS* p)
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_KILL_PROCESS;
   msg.id = p->getId();
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendCloneProcess(PROCESS* p)
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_CLONE_PROCESS;
   msg.id = p->getId();
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendStartProcess(const char_t *path)
{
   int ret=0;
   PROCESS_START msg;
   msg.msg.cmd = COMM_CMD_START_PROCESS;
   msg.msg.id = 0;
   msg.msg.sz = sizeof(PROCESS_START);
   wcscpy(msg.name, path);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.msg.sz, 0);
   return ret;
}

int DEVICE::sendRestartProcess(PROCESS* p)
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_RESTART_PROCESS;
   msg.id = p->getId();
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendKillThread(THREAD* t)
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_KILL_THREAD;
   msg.id = t->getId();
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendSuspendThread(THREAD* t)
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_SUSPEND_THREAD;
   msg.id = t->getId();
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);
   if(ret>0)
   {
      t->setSuspended(true);
   }
   return ret;
}

int DEVICE::sendResumeThread(THREAD* t)
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_RESUME_THREAD;
   msg.id = t->getId();
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   if(ret>0)
   {
      t->setSuspended(false);
   }
   return ret;
}

int DEVICE::sendSetThreadPriority(int tid, int priority)
{
   int ret=0;
   THREAD_PRIORITY msg;
   msg.msg.cmd = COMM_CMD_THREAD_PRIORITY;
   msg.msg.id = tid;
   msg.msg.sz = sizeof(THREAD_PRIORITY);
   msg.priority = priority;
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.msg.sz, 0);      
   return ret;
}

int DEVICE::sendResetDevice()
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_DEVICE_RESTART;
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendResetServer()
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_SERVER_RESTART;
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendCaptureOnDev(DWORD priority, BOOL capture_on_dev, DWORD interval_ms)
{
   int ret=0;
   CAPTURE_ON_DEV lmsg;
   lmsg.msg.cmd = COMM_CMD_CAPTURE_ON_DEV;
   lmsg.msg.sz = sizeof(lmsg);
   lmsg.capture_on_dev = capture_on_dev ? -1:0;
   lmsg.interval_ms = interval_ms;
   lmsg.priority = priority;
   ret = socket_write(m_so, (const unsigned char*)&lmsg, lmsg.msg.sz, 0);      
   return ret;
}

int DEVICE::sendSuspend()
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_DEVICE_SUSPEND;
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendShutdown()
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_DEVICE_SHUTDOWN;
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);  
   return ret;
}

int DEVICE::sendDisconnect()
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_CLOSE;
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);  
   return ret;
}


int DEVICE::sendGetSnapInfo()
{
   int ret=0;
   COMM_MESG msg;
   msg.cmd = COMM_CMD_GET_SNAPSHOT;
   msg.sz = sizeof(COMM_MESG);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.sz, 0);      
   return ret;
}

int DEVICE::sendDebugNewProcess(DWORD rqstid, const char_t *path)
{
   int ret=0;
   PROCESS_START msg;
   memset(&msg, 0, sizeof(PROCESS_START));
   msg.msg.cmd = COMM_CMD_DEBUG_NEW_PROCESS;
   msg.msg.sz = sizeof(PROCESS_START);
   msg.rqstid = rqstid;
   msg.pid = 0;
   wcscpy(msg.name, path);
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.msg.sz, 0);
   return ret;
}

int DEVICE::sendDebugActiveProcess(DWORD rqstid, DWORD pid)
{
   int ret=0;
   PROCESS_START msg;
   memset(&msg, 0, sizeof(PROCESS_START));
   msg.msg.cmd = COMM_CMD_DEBUG_ACTIVE_PROCESS;
   msg.msg.sz = sizeof(PROCESS_START);
   msg.rqstid = rqstid;
   msg.pid = pid;
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.msg.sz, 0);
   return ret;
}


int DEVICE::sendGetProcessPath(DWORD pid)
{
   int ret=0;
   FILE_DATA msg;
   memset(&msg, 0, sizeof(FILE_DATA));
   msg.msg.cmd = COMM_CMD_GET_PROCESS_PATH;
   msg.msg.sz = sizeof(FILE_DATA);
   msg.msg.id = pid;
   ret = socket_write(m_so, (const unsigned char*)&msg, msg.msg.sz, 0);
   return ret;
}

int DEVICE::sendSetProcessAffinity(DWORD pid, int affinity)
{
	int ret = 0;
	SET_CORE msg;
	msg.msg.cmd = COMM_CMD_SET_PROCESS_AFFINITY;
	msg.msg.id = pid;
	msg.msg.sz = sizeof(SET_CORE);
	msg.affinity = affinity;
	ret = socket_write(m_so, (const unsigned char*)&msg, msg.msg.sz, 0);
	return ret;
}

int DEVICE::sendSetThreadAffinity(DWORD tid, int affinity)
{
	int ret = 0;
	SET_CORE msg;
	msg.msg.cmd = COMM_CMD_SET_THREAD_AFFINITY;
	msg.msg.id = tid;
	msg.msg.sz = sizeof(SET_CORE);
	msg.affinity = affinity;
	ret = socket_write(m_so, (const unsigned char*)&msg, msg.msg.sz, 0);
	return ret;
}

void DEVICE::addSnapInfo(COMM_SNAPINFO *data)
{
   switch(m_index)
   {
   case 0:
      m_index=1;
      memcpy(&m_snapinfo[m_index], data, sizeof(COMM_SNAPINFO));
      break;
   case 1:
      m_index=2;
      memcpy(&m_snapinfo[m_index], data, sizeof(COMM_SNAPINFO));
      break;
   default:
      memcpy(&m_snapinfo[1], &m_snapinfo[2], sizeof(COMM_SNAPINFO));
      memcpy(&m_snapinfo[2], data, sizeof(COMM_SNAPINFO));

      if(m_mem_watch)
	  {
         checkMemWatch(data->memstatus.freeMemKB * 1024);
	  }
   }
}

long64 DEVICE::getTimeStamp(int index) const
{
   //return m_snapinfo[index].now;
   FILETIME t1 = m_snapinfo[index].ft;
   long64 ull1=0;
   ull1 = t1.dwHighDateTime;
   ull1 <<= 32;
   ull1 |= t1.dwLowDateTime;
   return ull1;
}

void DEVICE::getTimeStamp(SYSTEMTIME& st) const
{
   memset(&st, 0, sizeof st);
    st.wSecond = m_snapinfo[m_index].dt.tm_sec;         /* seconds */
    st.wMinute = m_snapinfo[m_index].dt.tm_min;         /* minutes */
    st.wHour = m_snapinfo[m_index].dt.tm_hour;        /* hours */
    st.wDay = m_snapinfo[m_index].dt.tm_mday;        /* day of the month */
    st.wMonth = m_snapinfo[m_index].dt.tm_mon + 1;         /* month */
    st.wYear = m_snapinfo[m_index].dt.tm_year + 1900;        /* year */
}

void DEVICE::checkMemWatch(DWORD available_mem)
{
   m_mem_watch->trigger(available_mem < m_mem_watch->m_min_mem*1024, this);
}

void DEVICE::checkCPUWatch(float cpuPercent)
{ 
	if (cpuPercent > 100)
		cpuPercent = 100;
   int i, j;
   WATCH_ITEM *watch = 0;
   for(i=0; i<m_proceses.GetCount(); i++)
   {
      PROCESS *p = (PROCESS*)m_proceses.GetAt(i);

      for(j=0; p && j<p->numThreads(); j++)
      {
         THREAD *t = p->getThread(j);

         if(t)
         {
            watch = t->getCPUWatch();
            if(watch)
            {
               float percent = t->cpuPercent(0);
               watch->trigger(percent > watch->m_max_cpu, this);
            }
         }

         watch = p->getCPUWatch();
         if(watch)
         {
            float percent = p->cpuPercent(0);
            watch->trigger(percent > watch->m_max_cpu, this);
         }
      }
   }

   if(m_cpu_watch)
   {
      m_cpu_watch->trigger(cpuPercent > m_cpu_watch->m_max_cpu, this);
   }
}


void DEVICE::delMemWatch()
{
   WATCH_ITEM *watch_item = getMemWatch();
   if(watch_item)
   {
      delete watch_item;
      setMemWatch(0);
   }
}

void DEVICE::delCPUWatch()
{
   int i, j;
   WATCH_ITEM *watch = 0;
   for(i=0; i<m_proceses.GetCount(); i++)
   {
      PROCESS *p = (PROCESS*)m_proceses.GetAt(i);

      for(j=0; j<p->numThreads(); j++)
      {
         THREAD *t = p->getThread(j);

         if(t)
         {
            watch = t->getCPUWatch();
            if(watch)
            {
               delete watch;
               t->setCPUWatch(0);
            }
         }

         watch = p->getCPUWatch();
         if(watch)
         {
            delete watch;
            p->setCPUWatch(0);
         }
      }
   }
}

void DEVICE::delTotalCPUWatch()
{
	WATCH_ITEM * watch_item = getCPUWatch();
	if(watch_item)
	{
		delete watch_item;
		setCPUWatch(0);
	}
}

const MY_MEM_INFO *DEVICE::getMemStatus() const
{
   /*logg(L"RAM: %i%%\nRAM %i/%i\nPageFile %i/%i\nVirtual %i/%i\n======\n",
   sdata->memstatus.dwMemoryLoad,
   sdata->memstatus.dwAvailPhys/1024/1024,
   sdata->memstatus.dwTotalPhys/1024/1024,
   sdata->memstatus.dwAvailPageFile/1024/1024,
   sdata->memstatus.dwTotalPageFile/1024/1024,
   sdata->memstatus.dwAvailVirtual/1024/1024,
   sdata->memstatus.dwTotalVirtual/1024/1024
   );*/
   switch(m_index)
   {
   case 0:
      return 0;
   default:
      return &m_snapinfo[m_index].memstatus;// - m_snapinfo[m_index-1].memstatus.dwMemoryLoad;
   }

}

PROCESS* DEVICE::getProcess(int i) const 
{ 
   return (PROCESS*)(m_proceses.GetAt(i)); 
}

PROCESS* DEVICE::removeProcess(int i)
{ 
   PROCESS* process = (PROCESS*)m_proceses.Remove(i);
   return process;
}

PROCESS* DEVICE::removeProcess(PROCESS* p)
{ 
   PROCESS* process = (PROCESS*)m_proceses.Remove(p);
   return process;
}

PROCESS* DEVICE::getProcessById(int id) const
{ 
   return m_proceses.getProcessById(id);
}

void DEVICE::close()
{
   int i;
   if(m_so)
   {
	   shutdown(m_so, SD_BOTH);
	   closesocket(m_so);
	  logg(TXT("closing socket %x\n"), m_so);
      m_so = 0;
   }

   for(i=0; i<m_proceses.GetCount(); i++)
   {
      PROCESS *p = (PROCESS*)m_proceses.GetAt(i);
      p->deref();
   }
   m_proceses.Destroy();

   cancelPsWatches();

   m_index=0;
   m_streamHead=0;
   m_streamTail=0;
}

int DEVICE::getProcessCount(bool includeThreads) const
{ 
   int count = m_proceses.GetCount(); 
   if(includeThreads)
   {
      int i;
      for(i=0; i<m_proceses.GetCount(); i++)
      {
         PROCESS* process = (PROCESS*)m_proceses.GetAt(i);
         count+=process->numThreads();
      }
   }
   return count;
}

void DEVICE::debugProcess(WATCH_ITEM* watch_item)
{
   if(watch_item)
   {
      m_watchlist.Insert(watch_item, 0);

      if(watch_item->m_debug_new_ps)
         sendDebugNewProcess((int)watch_item, watch_item->m_pspath.s());
      else if(watch_item->m_pid)
         sendDebugActiveProcess((int)watch_item, watch_item->m_pid);
   }
}

void DEVICE::debugProcessAckRecvd(DEBUG_PROCESS_ACK *ack)
{
   WATCH_ITEM* watch_item;
   if(!ack)
      goto Exit;

   watch_item = m_watchlist.getItem(ack->rqstid);
   if(!watch_item)
      goto Exit;

   if(!ack->msg.set)
   {
      m_watchlist.Remove(watch_item);
      watch_item->deref();

      py_crashwatch_ack(watch_item->m_pid, watch_item->m_pspath, false);
   }
   else
   {
      watch_item->m_pid = ack->pid;
      if(ack->pid)
      {
         PROCESS* p = getProcessById(ack->pid);
         if(p)
         {
            p->setCrashWatch(watch_item);
         }
         py_crashwatch_ack(watch_item->m_pid, watch_item->m_pspath, true);
      }
   }

Exit:
   return;
}

void DEVICE::cancelPsWatches()
{
   int i;
   for(i=0; i<m_watchlist.GetCount(); i++)
   {
      WATCH_ITEM *watch_item = (WATCH_ITEM*)m_watchlist.GetAt(i);
      watch_item->deref();
   }
   m_watchlist.Destroy();
}

void DEVICE::debugProcessCrashInfoRecvd(CRASH_INFO *cinfo)
{
   STRING msg;
   STRING subject;
   STRING mapfile;
   //PROCESS *p;
   WATCH_ITEM* watch_item;
   CDlgDbgPsInfo *dlg;

   if(!cinfo)
      goto Exit;

   //p = getProcessById(cinfo->debug_event.dwProcessId);

   watch_item = m_watchlist.getItem(cinfo->rqstid);
   if(watch_item)
      mapfile = watch_item->m_mapfile;

   dlg = new CDlgDbgPsInfo(this, cinfo, mapfile, 0);
   if(dlg)
      dlg->getExceptionMsg(msg);

   if(watch_item)
   {
	   py_got_exception(watch_item->m_pid, 
		  STRING(watch_item->m_pspath ? watch_item->m_pspath : L""), 
		  msg);
	}

   if(!watch_item || !watch_item->m_silent_mode)
   {
      if(dlg)
      {
         dlg->Create((UINT)IDD_DEBUG_PROCESS_ALERT, 0);
         dlg->m_edit_ps_info.SetWindowText(msg.size() ? msg.s() : L"");
         dlg->ShowWindow(SW_SHOW);
      }
      //if dlg is created, then it will be deleted in the OnDestroy
   }
   else if(dlg)
   {
      delete dlg;
      dlg = 0;
   }

   if(watch_item)
   {
      if(watch_item->m_emailto.size())
      {
         subject.set(L"Remote Task Monitor - A process on device has crashed");
         CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
         mainWnd->SendEmail(watch_item->m_emailto, subject, msg);
      }

      if(watch_item->m_reset_device)
      {
         sendResetDevice();
      }
      else if(watch_item->m_pspath2.size())
      {
         if(watch_item->m_watch_ps2)
         {
            WATCH_ITEM *watch_item2 = WATCH_ITEM::create(
               true, 0, 
               watch_item->m_pspath2.s(), 
               watch_item->m_mapfile.s(), 
               watch_item->m_emailto.s(),
               watch_item->m_pspath2.s(), 
               watch_item->m_watch_ps2, 
               watch_item->m_reset_device, 
               watch_item->m_silent_mode);
            if(watch_item2)
               debugProcess(watch_item2);
         }
         else
            sendStartProcess(watch_item->m_pspath2.s());
      }

   }
   /*   
   FILE* fp = fopen("crashlog.txt", "a+t");
   if(fp)
   {
   fprintf(fp, msg.ansi(), msg.size());
   fprintf(fp, "\n============================\r\n");
   fclose(fp);
   }
   */
Exit:
   return;
}

float DEVICE::powerPercent() const
{
	return m_snapinfo[m_index].power.battery_percent;
}

BOOL DEVICE::powerSourceIsAC() const
{
	return m_snapinfo[m_index].power.ac == 1;
}

DWORD DEVICE::powerSourceIsBattery() const
{
	return m_snapinfo[m_index].power.battery == 1;
}

float DEVICE::memPercent() const
{
   const MY_MEM_INFO *mem = &m_snapinfo[m_index].memstatus;
   float memLoad = (mem && mem->totalMemKB>0) ? (((mem->totalMemKB - mem->freeMemKB)*100.0)/(float)mem->totalMemKB) : 0;
   return memLoad;
}

STRING DEVICE::totalMem() const
{
	const MY_MEM_INFO *mem_info = &m_snapinfo[m_index].memstatus;
	STRING ret;
	ret.sprintf(TXT("%s"), PROCESS::memoryUsedHumanReadable(mem_info->totalMemKB * 1024));
	return ret;
}

STRING DEVICE::freeMem() const
{
	const MY_MEM_INFO *mem_info = &m_snapinfo[m_index].memstatus;
	STRING ret;
	ret.sprintf(TXT("%s"), PROCESS::memoryUsedHumanReadable(mem_info->freeMemKB * 1024));
	return ret;
}

STRING DEVICE::processArchitecture() const
{
	char2_t architecture[MAX_ARCH_SZ];
	memset(architecture, 0, sizeof(architecture));
	m_sysinfo.processorArchitecture(architecture, sizeof(architecture) / sizeof(char2_t));
	STRING ret(architecture);
	return ret;

}

STRING DEVICE::clockSpeed() const
{
	char2_t frequencyUnit[32] = TXT("KHz");
	double frequency = (double)m_sysinfo.frequency;
	if (m_sysinfo.frequency > 1000000)
	{
		frequency /= 1000000.0;
		strcpyW(frequencyUnit, TXT("GHz"));
	}
	else if (m_sysinfo.frequency > 1000)
	{
		frequency /= 1000.0;
		strcpyW(frequencyUnit, TXT("MHz"));
	}
	STRING ret;
	ret.sprintf(TXT("%.02f %s"), frequency, frequencyUnit);
	return ret;
}

long64 DEVICE::cpuIdleTime(bool avg, int core) const
{
   DWORD cput = 0;//milliseconds

   switch(m_index)
   {
   case 0:
      return 0;
   case 1:
      cput = (m_snapinfo[1].idle[core]  > m_snapinfo[0].idle[core]) ? m_snapinfo[1].idle[core] - m_snapinfo[0].idle[core] : 0;
   default:
      cput = avg ? 
         ((m_snapinfo[2].idle[core] > m_snapinfo[0].idle[core]) ? m_snapinfo[2].idle[core] - m_snapinfo[0].idle[core] : 0) :
         ((m_snapinfo[2].idle[core] > m_snapinfo[1].idle[core]) ? m_snapinfo[2].idle[core] - m_snapinfo[1].idle[core] : 0);
   }
   return cput;
}

long64 DEVICE::totalTimeDiff(bool avg)  const
{
   long64 ret = 0;
//   ret = (m_snapinfo[2].now - m_snapinfo[avg ? 0 : 1].now);

   switch(m_index)
   {
   case 0:
   case 1:
      return 0;
   default:
      ret = avg ? 
         getTimeStamp(2) - getTimeStamp(0):
         getTimeStamp(2) - getTimeStamp(1);
		break;
   }
   return ret/10000; // 100-ns to ms
}

float DEVICE::cpuPercentBySummingCoreCpu(bool avg, unsigned long core) const
{
	//return core == 0 ? cpuPercentBySummingProcessCpu(avg) : 0;//testing123
	if(m_sysinfo.frequency == 0)
	{
		logg(TXT("Invalid system frequency %i.\n"), m_sysinfo.frequency);
		return 0;
	}
	if(core >= MAX_CORES)
	{
		logg(TXT("Invalid core %i. Only %i cores are supported\n"), core, MAX_CORES);
		return 0;
	}
	if(core >= m_sysinfo.numberOfProcessors())
	{
		logg(TXT("Invalid core %i. Only %i cores were detected\n"), core, m_sysinfo.numberOfProcessors());
		return 0;
	}

	long64 tt = totalTimeDiff(avg);
	long64 cpuidle = cpuIdleTime(avg, core);
	float ret = calculatePercentage(tt, cpuidle);

	if(ret > m_max_cpu[core])
	{
		m_max_cpu[core] = ret;
	}
	//logg(L"cpu %i %.02f%%, cpuidle %I64d, tt %I64d\n", core, ret, cpuidle, tt);
	return ret;
}

float DEVICE::cpuPercentBySummingCoreCpu(bool avg) const
{
	if(m_sysinfo.frequency == 0)
	{
		logg(TXT("Invalid system frequency %i.\n"), m_sysinfo.frequency);
		return 0;
	}

	long64 tt = totalTimeDiff(avg);
	long64 cpuidle = 0;
	
	for(unsigned long core=0; core<m_sysinfo.numberOfProcessors(); core++)
	{
		cpuidle += cpuIdleTime(avg, core);
	}
	cpuidle /= m_sysinfo.numberOfProcessors();
	float ret = calculatePercentage(tt, cpuidle);
	//logg(L"cpu %.02f%%, cpuidle %I64d, tt %I64d\n", ret, cpuidle, tt);
	return ret;
}

float DEVICE::cpuPercentBySummingProcessCpu(bool avg) const
{
	float ret = 0;
	for (int i = 0; i < m_proceses.GetCount(); i++)
	{
		PROCESS *p = (PROCESS*)m_proceses.GetAt(i);
		ret += p->cpuPercent(avg);
	}
	return ret;
}

float DEVICE::cpuPercent(bool avg) const
{
	if (1 || m_sysinfo.numberOfProcessors() == 1)
	{
		return cpuPercentBySummingProcessCpu(avg);
	}
	else
	{
		return cpuPercentBySummingCoreCpu(avg);
	}
}

int DEVICE::numCores() const
{
	return m_sysinfo.numberOfProcessors();
}
