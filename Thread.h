#ifndef THREAD_H
#define THREAD_H
//

#include "stdafx.h"
#include "../ds/ds.h"
#include "remotecpu.h"
#include "ExecObj.h"

class PROCESS;

#pragma warning (disable : 4996 )//wcscpy_s

class THREAD : public EXECOBJ
{
public:
   static THREAD* create(EXECOBJ_DATA *data, PROCESS *p);
   char_t* getName() { return m_name; }
   void setName(const char_t* name) { wcscpy(m_name, name); }
   DWORD getId() { return m_data[0].tid; }
   void setId(unsigned long id) { m_data[0].tid = m_data[1].tid = m_data[2].tid = id; }
   void addData(EXECOBJ_DATA* data);
   PROCESS* getProcess() const { return m_process; }
   int getPriority();
   long64 cpuTime(bool avg);
   long64 totalTimeDiff(bool avg);
   float cpuPercent(bool avg);
   void setSuspended(char val) { m_suspended = val; }
   char getSuspended() const { return m_suspended; }
   void setAffinity(int core) const;
   THREAD(DEVICE* device) : EXECOBJ(EXECOBJ_THREAD, device) {Init();}
   int affinity() const;
private:
   THREAD(EXECOBJ_DATA *data, PROCESS *p);
   void Init();
   EXECOBJ_DATA m_data[3];
   PROCESS *m_process;
   char m_suspended;
   bool m_tagged;//used by history
   char2_t m_name[64];
};

#endif//THREAD_H