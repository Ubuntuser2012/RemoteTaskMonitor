#include "stdafx.h"
#include "../common/types.h"
extern "C" {
#include "comm.h"
}
#include "../fileio/fileio.h"
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "devicelist.h"
#include "datapoint.h"
#include "gdgraph.h"
#include "history.h"
#include "dlgRemotecpu.h"
#include "raphael.h"
#include "dialog2.h"
#include "../common/util.h"

#define PADDING_LINES 512
static char_t padding[] = 
#include "res/padding.txt"

//These max numbers are there so the m_history_tags does not keep growing 
//as new processes and threads are started.
//The m_history_tags does not purge itself of the old processes and threads
//so it can grow like crazy
#define MAX_PROCESSES 200
#define MAX_THREADS_PER_PROCESS 300

DATA_QUE::DATA_QUE(int max, STRING &out_dir)
:  QUE()
, m_fp(0)
, m_first(true)
{
	Init(max, out_dir);
}


int DATA_QUE::Init(int max, STRING &out_dir)
{
	SetMax(max);
	if(out_dir.size())
	{
		m_first = true;
		m_outdir.set(out_dir.s());
		m_file.sprintf(L"%s\\%x.txt", out_dir.s(), this);
		m_fp = fopen(m_file.ansi(), "w+t");
		if(!m_fp)
		{
			int x=0;
			x=1;
		}
		return true;
	}
	else
	{
		return false;
	}
}

DATA_QUE::~DATA_QUE()
{
	DATA_VALUE *dv = 0;
	
	while(1)
	{
		dv = (DATA_VALUE *)Deque();
		if(!dv)
			break;
		delete dv;
	}
	Destroy();

	if(m_fp)
	{
		fclose(m_fp);
		m_fp = 0;
	}
	m_first = true;
}

STRING &DATA_QUE::labels()
{
	if(m_first)
	{
		char_t tmp[1024];
		DATA_VALUE *dv = 0;
		int i;
		
		m_str.set(L"");
		for(i=0;; i++)
		{
			dv = (DATA_VALUE *)Deque();
			if(!dv)
				break;
			else
			{
				swprintf(tmp, L"%s\"%04i/%02i/%02i\\n%02i:%02i:%02i:%03i\"", 
					i==0 ? L"" : L", ",
					dv->u.ts.wYear,
					dv->u.ts.wMonth,
					dv->u.ts.wDay,
					dv->u.ts.wHour,
					dv->u.ts.wMinute,
					dv->u.ts.wSecond,
					dv->u.ts.wMilliseconds);
				m_str.append(tmp);
				delete dv;
			}
		}
	}
	else
	{
		readf();
	}
   return m_str;
}

STRING &DATA_QUE::cpu()
{
	if(m_first)
	{
		char_t tmp[1024];
		DATA_VALUE *dv = 0;
		int i;
		
		m_str.set(L"");
		for(i=0;; i++)
		{
			dv = (DATA_VALUE *)Deque();
			if(!dv)
				break;
			else
			{
				swprintf(tmp, L"%s\"%.02f\"", 
					i==0 ? L"" : L", ",
					dv->u.fl);
				m_str.append(tmp);
				delete dv;
			}
		}
	}
	else
	{
		readf();
	}
	return m_str;
}

STRING &DATA_QUE::mem()
{
	if(m_first)
	{
		char_t tmp[1024];
		DATA_VALUE *dv = 0;
		int i;
		
		m_str.set(L"");
		for(i=0;; i++)
		{
			dv = (DATA_VALUE *)Deque();
			if(!dv)
				break;
			else
			{
				swprintf(tmp, L"%s\"%u\"", 
					i==0 ? L"" : L", ",
					dv->u.dw);
				m_str.append(tmp);
				delete dv;
			}
		}
	}
	else
	{
		readf();
	}
	return m_str;
}

int DATA_QUE::readf()
{
	int ret = 0;
	int sz = ftell(m_fp);
	if(sz>0)
	{
		char* buffer = new char[sz+sizeof(char_t)];
		if(buffer)
		{
			fseek(m_fp, 0, SEEK_SET);
			fread(buffer, sz, 1, m_fp);
			buffer[sz]=0;
			m_str.set(buffer);
			delete [] buffer;
			m_first = true;
			ret = 1;
		}
	}
	fclose(m_fp);
	m_fp=0;
	DeleteFile(m_file);
	return ret;
}

int DATA_QUE::Enque(DWORD dw)
{
	STRING tmp;
	
	tmp.sprintf(L"%s\"%u\"", 
		m_first ? L"" : L", ",
		dw);
	if(m_fp)
	{
		fwprintf(m_fp, tmp.s());
	}
	
	m_first = false;
	return 1;
}

int DATA_QUE::Enque(float fl)
{
	STRING tmp;
	
	tmp.sprintf(L"%s\"%.02f\"", 
        m_first ? L"" : L", ",
        fl);

	if(m_fp)
	{
		fwprintf(m_fp, tmp.s());
	}	
	m_first = false;
	return 1;
}

int DATA_QUE::Enque(SYSTEMTIME &ts)
{
	STRING tmp;
	
	tmp.sprintf(L"%s\"%04i/%02i/%02i\\n%02i:%02i:%02i:%03i\"", 
        m_first ? L"" : L", ",
        ts.wYear,
        ts.wMonth,
        ts.wDay,
        ts.wHour,
        ts.wMinute,
        ts.wSecond,
        ts.wMilliseconds);

	if(m_fp)
	{
		fwprintf(m_fp, tmp.s());
	}
	
	m_first = false;
	return 1;
}

int DATA_QUE::Enque(OBJECT* p)
{
	int max = GetMax();
	int count = GetCount();
	if(count > max*60)
	{
		DATA_VALUE *dv = (DATA_VALUE *)Deque();
		if(dv)
		{
			delete dv;
		}
	}
	return QUE::Enque(p);
}

HISTORY_DATA::HISTORY_DATA(GDGraph &gd, STRING &out_dir, DWORD pid, const char2_t *pname, DWORD tid, char_t *tname, unsigned char priority)
: m_pid(pid)
, m_tid(tid)
, m_priority(priority)
, m_gd(gd)
, m_cpu(m_gd, out_dir, L"cpu", pid, tid)
, m_mem(m_gd, out_dir, L"mem", pid, tid)
{ 
   m_pname[0]=0;
   m_tname[0]=0;
   if(pname)
      wcscpy(m_pname, pname);
   if(tname)
      wcscpy(m_tname, tname);
   m_cpu.SetDir(out_dir);
   m_mem.SetDir(out_dir);
}

HISTORY::HISTORY(CDlgRemoteCPU *parent)
: m_parent(parent)
, m_num_writes(0)
, m_state(HISTORY_STATE_STOPPED)
, m_gd()
, m_cpu(m_gd, m_outdir.s(), L"total-cpu", 0, 0)
, m_mem(m_gd, m_outdir.s(), L"total-mem", 0, 0)
, m_dir()
{
}

DATA_LIST::DATA_LIST(GDGraph &gd, const char_t* outdir, const char_t* fileprefix, DWORD pid, DWORD tid) 
: m_count(0)
, m_reset_count(0)
, m_pid(pid)
, m_tid(tid)
, m_fp(0)
, m_gd(gd)
{ 
   m_outdir.set(outdir);
   if(fileprefix)
      wcscpy(m_fileprefix, fileprefix);
   else
      m_fileprefix[0]=0;
}

DATA_LIST::~DATA_LIST()
{
   if(m_fp)
      fclose(m_fp);
}

bool DATA_LIST::Insert(float val, SYSTEMTIME &ts)
{
	bool ret = false;
	if(m_count<MAX_DATA_LIST_SIZE)
	{
		m_data[m_count].ts = ts;
		m_data[m_count].val = val;
		m_count++;
		ret = true;
	}
	if(m_count==MAX_DATA_LIST_SIZE)
	{
		ret = Flush(0);
	}
	return ret;
}

void DATA_LIST::SetDir(STRING &outdir)
{
   if(m_pid && m_tid && m_fileprefix[0]==L'c')//cpu of thread
   {
      m_outdir.sprintf(L"%s%s-pid-%i-tid-%i",//.dat", 
         outdir.s(), 
         *m_fileprefix ? m_fileprefix : L"",
         m_pid, 
         m_tid
         );
   }
   else if(m_pid && !m_tid && m_fileprefix[0]==L'c')//cpu of process
   {
      m_outdir.sprintf(L"%s%s-pid-%i",//.dat", 
         outdir.s(), 
         *m_fileprefix ? m_fileprefix : L"",
         m_pid 
         );
   }
   else if(m_pid && !m_tid && m_fileprefix[0]==L'm')//mem of process
   {
      m_outdir.sprintf(L"%s%s-pid-%i",//.dat", 
         outdir.s(), 
         *m_fileprefix ? m_fileprefix : L"",
         m_pid 
         );
   }
   else if(!m_pid && !m_tid)//cpu total or mem total
   {
      m_outdir.sprintf(L"%s%s",//.dat", 
         outdir.s(), 
         *m_fileprefix ? m_fileprefix : L""
         );
   }

   if(m_outdir.size())
   {
      //m_fp = fopen(m_outdir.ansi(), "wb");
   }

   m_reset_count = 0;
}

bool DATA_LIST::Flush(bool close)
{
	bool ret = false;
	
   if(m_fp)
   {
      fwrite(m_data, sizeof(m_data), 1, m_fp);
      if(close)
      {
         fclose(m_fp);
         m_fp = 0;
      }
	  ret = 1;
   }
   else if(m_outdir.size())
   {
		STRING outfile;
		outfile.sprintf(L"%s-%i.png", m_outdir.s(), m_reset_count);
		logg(TXT("enque data %s\n"), outfile.s());
	    ret = m_gd.gdEnque(outfile, m_data, sizeof(m_data));
   }

	m_count=0; 
	m_reset_count++;
	
	return ret;
}

HISTORY::~HISTORY()
{
	STRING outdir;
	Stop(0, outdir, 0);
}

STRING& HISTORY::GetDirectory()
{
	return m_dir;
}

void HISTORY::SetDirectory(STRING& dir)
{
	m_dir.set(dir.s());
}

bool HISTORY::PrependTempFilesWhiteSpace()
{
   STRING *fname;
   FILEIO fp;
   bool ret = false;
   int k;
   char_t msg[1024];

   if(1)
   {
      ret = true;
      int i;
      m_history_fname[HISTORY_FILE_PS].append(L'~');
      m_history_fname[HISTORY_FILE_TH].append(L'~');
      m_history_fname[HISTORY_FILE_MEM].append(L'~');
      for(k=0; k<HISTORY_NUM_FILES; k++)
      {
         fname = &m_history_fname[k];

         if(!fp.open(fname->s(), FILEIO_MODE_WRITE|FILEIO_MODE_OPEN_EXISTING) &&
            !fp.open(fname->s(), FILEIO_MODE_WRITE|FILEIO_MODE_CREATE_ALWAYS))
         {
            swprintf(msg, L"Failed to open file %s", fname->s());
            //m_parent->MessageBox(msg, L"Error", MB_ICONEXCLAMATION);
            ret = false;
            break;
         }
         else
         {
            //fp.write(buffer, wcslen(buffer)*sizeof(char_t));
            for(i=0; i<PADDING_LINES; i++)
            {
               //need progress bar here
               fp.write(padding, sizeof(padding));
            }
            fp.printf(L"\r\n");
            fp.close();
         }

      }
      //remove the ~ that was appended to the name
      m_history_fname[HISTORY_FILE_PS].setAt(m_history_fname[HISTORY_FILE_PS].size()-1, L'\0');
      m_history_fname[HISTORY_FILE_TH].setAt(m_history_fname[HISTORY_FILE_TH].size()-1, L'\0');
      m_history_fname[HISTORY_FILE_MEM].setAt(m_history_fname[HISTORY_FILE_MEM].size()-1, L'\0');
   }

   return ret;
}

bool HISTORY::Start(const char_t *filename)
{
   bool ret = false;
   int k;
   STRING dt, tmp;
   STRING *fname;
   STRING path;
   //STRING padding;
   SYSTEMTIME sysTime;
   GetLocalTime(&sysTime);

   int key = m_parent->GetLicenseType();

   if(filename)
   {
      STRING arr[4];
      STRING _fname;
      int num_strings = 4;
      _fname.set(filename);
      _fname.explode(L"[]", arr, num_strings);
      if(arr[2].size()>=MAX_DIR_PATH)
         goto Exit;
      dt.sprintf(L"[%s]", arr[2].s());
   }
   else
   {
      dt.sprintf(L"[%04i-%02i-%02i--%02i-%02i-%02i-%03i]",
         sysTime.wYear,
         sysTime.wMonth,
         sysTime.wDay,
         sysTime.wHour,
         sysTime.wMinute,
         sysTime.wSecond,
         sysTime.wMilliseconds);
   }

   if(m_dir.s() && m_dir.size()>0)
	   path.set(m_dir.s());
   else if(!m_parent->GetLocalAppDataDirectory(path))
      goto Exit;

   if(!CreateDirectory(path.s(), 0)&& ERROR_ALREADY_EXISTS!=GetLastError())
   {
      goto Exit;
   }
   path.append(L"\\logs");
   if (!CreateDirectory(path.s(), 0) && ERROR_ALREADY_EXISTS != GetLastError())
   {
	   goto Exit;
   }
   

   path.append(L"\\");
   path.append(dt.s());
   if(!CreateDirectory(path.s(), 0)&& ERROR_ALREADY_EXISTS!=GetLastError())
   {
      goto Exit;
   }

   for(k=0; k<HISTORY_NUM_FILES; k++)
   {
      fname = &m_history_fname[k];
      fname->set(path.s());

      switch(k) 
      {
      case HISTORY_FILE_PS:
         tmp.sprintf(L"\\log%s.csv", L".ps");
         break;
      case HISTORY_FILE_TH:
         tmp.sprintf(L"\\log%s.csv", L".th");
         break;
      case HISTORY_FILE_MEM:
         tmp.sprintf(L"\\log%s.csv", L".mem");
         break;
      }
      fname->append(tmp.s());
      if(k==HISTORY_FILE_PS)
      {
         m_outdir.set(path.s());
         m_outdir.append(L"\\");

         int log_duration = m_parent->GetLogDuration() * 60;

         m_cpu.SetDir(m_outdir);
         m_mem.SetDir(m_outdir);
      }
   }

   PrependTempFilesWhiteSpace();
#if 0
   {
      m_history_fname[HISTORY_FILE_PS].append(L'~');
      m_history_fname[HISTORY_FILE_TH].append(L'~');
      m_history_fname[HISTORY_FILE_MEM].append(L'~');
      for(k=0; k<HISTORY_NUM_FILES; k++)
      {
         fname = &m_history_fname[k];
         CopyFile(padding.s(), fname->s(), true);
      }
      m_history_fname[HISTORY_FILE_PS].setAt(m_history_fname[HISTORY_FILE_PS].size()-1, L'\0');
      m_history_fname[HISTORY_FILE_TH].setAt(m_history_fname[HISTORY_FILE_TH].size()-1, L'\0');
      m_history_fname[HISTORY_FILE_MEM].setAt(m_history_fname[HISTORY_FILE_MEM].size()-1, L'\0');
   }
#endif
   m_num_writes = 0;
   m_state = HISTORY_STATE_STARTED;

   m_gd.gdStart();

   ret = true;
Exit:
   return ret;
}

bool HISTORY::Stop(DEVICE *device, STRING &outdir, CDlgRemoteCPU* parent)
{
   bool ret = false;
   int k,pi=0;
   STRING *fname=0;
   FILEIO fp1, fp2;
   char_t msg[1024];
   const char *s = 0;
   int sz = 0;
   char buffer[10240];
   DWORD nbytes;
   Raphael r;
   int num_processes;
   int num_threads;
   ITERATOR it;
   HISTORY_DATA *h;
   
   int key = m_parent->GetLicenseType();

   if(!device)
      goto Exit;

   if(m_state == HISTORY_STATE_STOPPED)
   {
      goto Exit;
   }
   if(!m_history_fname[0][0])
   {
      goto Exit;
   }

   num_processes = m_history_tags[0].GetCount();
   num_threads = m_history_tags[1].GetCount();

   for(k=0; k<HISTORY_NUM_FILES; k++)
   {
      fname = &m_history_fname[k];
      if(fname->size())
      {
         if(!fp1.open(fname->s(), FILEIO_MODE_WRITE|FILEIO_MODE_READ|FILEIO_MODE_OPEN_EXISTING))
         {
            swprintf(msg, L"Failed to open file %s", fname->s());
            //m_parent->MessageBox(msg, L"Error", MB_ICONEXCLAMATION);
            goto Exit;
         }
         else
         {
            bool deleteTempFile = true;
            m_history_fname[k].append(L'~');
            fname = &m_history_fname[k];

            if(!fp2.open(fname->s(), FILEIO_MODE_WRITE|FILEIO_MODE_READ|FILEIO_MODE_OPEN_EXISTING))
            {
               swprintf(msg, L"Failed to open file %s", fname->s());
               //m_parent->MessageBox(msg, L"Error", MB_ICONEXCLAMATION);
               goto Exit;
            }
            else
            {
               if(fp1.size() < PADDING_LINES*sizeof(padding))
               {
                  fp1.seek(0, FILE_BEGIN);

                  while(1)
                  {
                     nbytes = fp1.read(buffer, sizeof(buffer));
                     if(nbytes==0)
                     {
                        break;
                     }
                     fp2.write(buffer, nbytes);
                  }

                  fp2.seek(0, FILE_END);

                  deleteTempFile = false;
               }
               else
               {
                  fp1.seek(0, FILE_END);

                  while(1)
                  {
                     nbytes = fp2.read(buffer, sizeof(buffer));
                     if(nbytes==0)
                     {
                        break;
                     }
                     fp1.write(buffer, nbytes);
                  }

               }
               fp2.close();

            }
            fp1.close();

            if(deleteTempFile)
            {
               DeleteFile(fname->s());
            }
            else
            {
               STRING tmpFile;
               tmpFile.set(fname->s());
               fname->setAt(fname->size()-1, L'\0');
               DeleteFile(fname->s());
               MoveFile(tmpFile.s(), fname->s());
            }
         }
      }
   }//for

   outdir=m_outdir;

   ret = true;
Exit:

   m_cpu.Flush(1);
   m_mem.Flush(1);

   for(k=0; k<HISTORY_NUM_FILES-1 && device; k++)
   {
      h = (HISTORY_DATA*)m_history_tags[k].GetFirst(it);
      while(h && device)
      {
         PROCESS *p = device->getProcessById(h->m_pid);
         if(k==HISTORY_FILE_PS)
         {
            if(p && p->getGenReport())
            {
               h->m_cpu.Flush(1);
               h->m_mem.Flush(1);
            }
         }
         else if(k==HISTORY_FILE_TH)
         {
            THREAD *t = p ? p->getThreadById(h->m_tid) : 0;
            if(t && t->getGenReport())
            {
               h->m_cpu.Flush(1);
            }
         }
         h = (HISTORY_DATA*)m_history_tags[k].GetNext(it);
      }
   }
   m_state = HISTORY_STATE_STOPPED;

   if(device && parent && parent->m_hWnd)
   {
      GenerateReport(device, parent);
   }

   for(k=0; k<HISTORY_NUM_FILES-1 && device; k++)
   {
      h = (HISTORY_DATA*)m_history_tags[k].GetFirst(it);
      while(h)
	  {
		  delete h;
		  h = (HISTORY_DATA*)m_history_tags[k].GetNext(it);
	  }
      m_history_tags[k].Destroy(0);
   }
   return ret;
}

bool HISTORY::Write(DEVICE *device, CDlgRemoteCPU* parent)
{
   bool ret = false;
   FILEIO fp;
   int i,j,k;
   STRING *fname;
   wchar_t msg[1024];
   float cpuPercent=0;
   PROCESS *p;
   THREAD *t;
   SYSTEMTIME ts;
   MY_SYSTEM_INFO *si=0;
   STRING sysinfo, memtotal, memavail;
   HISTORY_DATA *h;
   ITERATOR it;
   int num_ps = 0;
   int num_th = 0;
   int num_dead_ps = 0;
   int num_dead_th = 0;

   int key = m_parent->GetLicenseType();

#ifdef PERF_LOG
   DWORD start = GetTickCount(), tc = 0, now = 0;//$
   FILE* log = fopen("history.log", "a+t");//$
   if(!log)
   {
      int x=0;
      x=1;
   }
#endif
   if(device)
   {
	  const MY_MEM_INFO *mem = device->getMemStatus();
      si = device->getSysInfo();
	  device->getTimeStamp(ts);

	  if(!mem || !si)
		  goto Exit;

      m_history_fname[HISTORY_FILE_PS].append(L'~');
      m_history_fname[HISTORY_FILE_TH].append(L'~');
      m_history_fname[HISTORY_FILE_MEM].append(L'~');
      
#ifdef PERF_LOG
      tc = GetTickCount();//$
#endif      
      for(i=0; i<m_history_tags[0].GetCount(); i++)
      {
         h = (HISTORY_DATA*)m_history_tags[0].GetAt(i);
         if(h)
         {
            p = device->getProcessById(h->m_pid);
            if(p)
            {
               cpuPercent+=p->cpuPercent(0);
            }
         }
      }
#ifdef PERF_LOG
      now = GetTickCount();//$
      if(log) fprintf(log, "#ps:%i delta:%i\t", device->getProcessCount(0), now - tc);//$
#endif

      WriteTempData(device->memPercent(), cpuPercent, ts);

      for(k=0; k<HISTORY_NUM_FILES; k++)
      {
         fname = &m_history_fname[k];

         if(!fp.open(fname->s(), FILEIO_MODE_WRITE|FILEIO_MODE_OPEN_EXISTING) &&
            !fp.open(fname->s(), FILEIO_MODE_WRITE|FILEIO_MODE_CREATE_ALWAYS))
         {
            swprintf(msg, L"Failed to open file %s", fname->s());
            //m_parent->MessageBox(msg, L"Error", MB_ICONEXCLAMATION);
            ret = false;
            break;
         }
         else
         {
            fp.seek(0, FILE_END);

            fp.printf(L"%04i/%02i/%02i-%02i:%02i:%02i:%03i", 
				   ts.wYear,
				   ts.wMonth,
				   ts.wDay,
				   ts.wHour,
				   ts.wMinute,
				   ts.wSecond,
				   ts.wMilliseconds);

            fp.printf(L"\t%u", device->memPercent());
            fp.printf(L"\t%.02f", cpuPercent);

#ifdef PERF_LOG
            tc = GetTickCount();//$
#endif
            h = (HISTORY_DATA*)m_history_tags[k%2].GetFirst(it);
            while(h)
            {
               p = device->getProcessById(h->m_pid);
               if(!p)
               {
                  fp.printf(L"\t0");//process is no longer alive
                  num_dead_ps++;
               }
               else
               {
                  if(k==HISTORY_FILE_PS)
                  {
                     p->setTag(TAG_TYPE_HISTORY);
                     fp.printf(L"\t%.02f", p->cpuPercent(0));

                     WriteTempData(h, p, 0, ts, mem);

                  }
                  else if(k==HISTORY_FILE_TH)
                  {
                     t = p->getThreadById(h->m_tid);
                     if(!t)
                     {
                        fp.printf(L"\t0");//thread is no longer alive
                        num_dead_th++;
                     }
                     else
                     {
                        t->setTag(TAG_TYPE_HISTORY);
                        fp.printf(L"\t%.02f", t->cpuPercent(0));

                        WriteTempData(h, p, t, ts, mem);

                     }
                  }
                  else if(k==HISTORY_FILE_MEM)
                  {
                     fp.printf(L"\t%i", p->memoryUsed());
                  }
               }
               h = (HISTORY_DATA*)m_history_tags[k%2].GetNext(it);
            }
#ifdef PERF_LOG
            now = GetTickCount();//$
            if(log) fprintf(log, "k:%i #dps:%i #dth:%i delta:%i\t", k, num_dead_ps, num_dead_th, now - tc);//$
            tc = GetTickCount();//$
#endif
            num_ps = device->getProcessCount(0);
            for(i=0; i<num_ps; i++)
            {
               p = device->getProcess(i);
               if(k==HISTORY_FILE_PS)
               {
                  if(!p->isTagged(TAG_TYPE_HISTORY) && p->getId() && m_history_tags[k].GetCount()<MAX_PROCESSES)
                  {
                     h = new HISTORY_DATA(m_gd, m_outdir, p->getId(), p->getName(), 0, 0, 0);
                     if(h)
                     {
                        m_history_tags[k%2].Insert(h);
                        fp.printf(L"\t%.02f", p->cpuPercent(0));

                        WriteTempData(h, p, 0, ts, mem);

                     }
                     p->setTag(TAG_TYPE_HISTORY);
                  }
               }
               else if(k==HISTORY_FILE_TH)
               {
                  num_th += p->numThreads();
                  for(j=0; j<p->numThreads(); j++)
                  {
                     t = p->getThread(j);
                     if(t)
                     {
                        if(!t->isTagged(TAG_TYPE_HISTORY) && m_history_tags[k].GetCount()<MAX_THREADS_PER_PROCESS)
                        {
                           h = new HISTORY_DATA(m_gd, m_outdir, p->getId(), p->getName(), t->getId(), t->getName(), t->getPriority());
                           if(h)
                           {
                              m_history_tags[k%2].Insert(h);
                              fp.printf(L"\t%.02f", t->cpuPercent(0));

                              WriteTempData(h, p, t, ts, mem);

                           }
                           t->setTag(TAG_TYPE_HISTORY);
                        }
                     }
                  }
               }
            }
#ifdef PERF_LOG
            now = GetTickCount();//$
            if(log) fprintf(log, "k:%i #ps:%i #th:%i delta:%i\t", k, num_ps, num_th, now - tc);//$
#endif
            device->resetTag(TAG_TYPE_HISTORY);
            fp.printf(L"\r\n");

            fp.close();
            ret = true;
         }
      }

      //remove the ~ that was appended to the name
      m_history_fname[HISTORY_FILE_PS].setAt(m_history_fname[HISTORY_FILE_PS].size()-1, L'\0');
      m_history_fname[HISTORY_FILE_TH].setAt(m_history_fname[HISTORY_FILE_TH].size()-1, L'\0');
      m_history_fname[HISTORY_FILE_MEM].setAt(m_history_fname[HISTORY_FILE_MEM].size()-1, L'\0');
   }

   m_num_writes++;
#ifdef PERF_LOG
   now = GetTickCount();//$
#endif
   Tags();
#ifdef PERF_LOG
   if(log) fprintf(log, "tags delta:%i\t", now - tc);//$
#endif
Exit:
#ifdef PERF_LOG
   now = GetTickCount();//$
   if(log) fprintf(log, "delta:%i\n", now - start);//$
   if(log) fclose(log);
#endif
   return ret;
}

bool HISTORY::Tags()
{
   bool ret = false;
   FILEIO fp;
   int k;
   STRING *fname;
   wchar_t msg[1024];
   HISTORY_DATA *h;
   ITERATOR it;

   for(k=0; k<HISTORY_NUM_FILES; k++)
   {
      fname = &m_history_fname[k];

      if(!fp.open(fname->s(), FILEIO_MODE_WRITE|FILEIO_MODE_OPEN_EXISTING) &&
         !fp.open(fname->s(), FILEIO_MODE_WRITE|FILEIO_MODE_CREATE_ALWAYS))
      {
         swprintf(msg, L"Failed to open file %s", fname->s());
         //m_parent->MessageBox(msg, L"Error", MB_ICONEXCLAMATION);
         ret = false;
         break;
      }
      else
      {
         fp.seek(0, FILE_BEGIN);
         fp.printf(L"TimeStamp\tMemory%%\tCPU%%");
         
         h = (HISTORY_DATA*)m_history_tags[k%2].GetFirst(it);
         while(h)
         {
            if(k==HISTORY_FILE_PS)
            {
               fp.printf(L"\t[%i]%s", h->m_pid, h->m_pname);
            }
            else if(k==HISTORY_FILE_TH)
            {
               fp.printf(L"\t[pid:%i]%s[tid:%i]%s[priority:%u]", 
                  h->m_pid, 
                  wcslen(h->m_pname) ? h->m_pname : L"",
                  h->m_tid, 
                  wcslen(h->m_tname) ? h->m_tname : L"",
                  h->m_priority);
            }
			else if(k==HISTORY_FILE_MEM)
            {
               fp.printf(L"\t[%i]%s", h->m_pid, h->m_pname);
            }

            h = (HISTORY_DATA*)m_history_tags[k%2].GetNext(it);
         }

         fp.printf(L"\n");
         fp.seteof();
         fp.close();
         ret = true;
      }
   }
   return ret;
}

bool HISTORY::WriteTempData(HISTORY_DATA *h, PROCESS* p, THREAD* t, SYSTEMTIME &ts, const MY_MEM_INFO *mem)
{
   bool ret = 0;
   if(p && t && t->getGenReport())
   {
      h->m_cpu.Insert(t->cpuPercent(0), ts);
   }
   else if(p && p->getGenReport())
   {
      float cpu = p->cpuPercent(0);
      float me = 100.0f * p->memoryUsed() / (float)mem->totalMemKB / 1024.0f;
      h->m_cpu.Insert(cpu, ts);
      h->m_mem.Insert(me, ts);
   }
   ret = 1;
   return ret;
}

bool HISTORY::WriteTempData(DWORD memoryLoad, float cpu, SYSTEMTIME &ts)
{
   m_cpu.Insert(cpu, ts);
   m_mem.Insert((float)memoryLoad, ts);
   return true;
}

void HISTORY::GenerateReport(DEVICE *device, CDlgRemoteCPU* parent)
{
   CDialog2 dlg;
   STRING title;
   CWnd *progressWnd, *staticWnd;
   DWORD i;
   int j=1;
   int   total = m_gd.gdRemaining();
   if(parent && parent->m_hWnd && dlg.Create(IDD_LOADING_DATA, (CWnd*)parent))
   {
      if(total>0)
      {
         progressWnd = dlg.GetDlgItem(IDC_PROGRESS_LOAD_DATA);
         staticWnd = dlg.GetDlgItem(IDC_STATIC_LOADING_RTM);

         dlg.ShowWindow(SW_SHOW);

         title.set(L"   ");
         title.sprintf(L"Generating graphs. Please wait...");
         staticWnd->SetWindowText(title.s());
         j=1;
         i=GetTickCount();

         for(; j && total;)
         {
            if(GetTickCount()-i>500)
            {
               j = m_gd.gdRemaining();
               if(!dlg.IsClosed())
                  progressWnd->SendMessage(PBM_SETPOS, (total-j)*100/total+1);
               i = GetTickCount();
            }

            if(1)
            {
               int ret1 = parent->ProcessWinMessages(&dlg);
               switch(ret1)
               {
               case -1:
                  break;
               case 0:
                  j=0;//force a break in the while loop
                  //case 1://we processed a message, continue doing that
               }
            }
            
            if(dlg.IsClosed())
            {
               m_gd.gdStop();
            }
         }
      }
      //////
      if(device && !dlg.IsClosed())
      {
         m_gd.gdReport(dlg, device, m_outdir, parent, m_history_tags, m_cpu, m_mem);
      }
      dlg.DestroyWindow();
   }
   m_gd.gdStop();
}
