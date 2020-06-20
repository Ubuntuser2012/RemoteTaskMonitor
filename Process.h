#ifndef PROCESS_H
#define PROCESS_H
// ProcessList.cpp : implementation file
//

#include "stdafx.h"
#include "../ds/ds.h"
#include "remotecpu.h"
#include "ExecObj.h"

class THREAD;
class DEVICE;

class PROCESS : public EXECOBJ
{
public:
	static PROCESS* create(EXECOBJ_DATA *data, DEVICE *device);
	const char2_t* getName() { return m_data[0].name; }
	DWORD getId() { return m_data[0].pid; }
	void setId(unsigned long id) { m_data[0].pid = m_data[1].pid = m_data[2].pid = id; }
	THREAD *addThread(EXECOBJ_DATA *data);
	void addData(EXECOBJ_DATA* data);
	float cpuPercent(bool avg);
	int affinity() const;
	unsigned long memoryUsed() const;
	static STRING memoryUsedHumanReadable(unsigned long memused);
	void close();
	int numThreads() { return m_threads.GetCount(); }
	THREAD* getThread(int i) { return (THREAD*)m_threads.GetAt(i); }
	THREAD* getThreadById(int id) { return (THREAD*)m_threads.getThreadById(id); }
   void removeUntaggedThreads() { m_threads.removeUntaggedThreads(); }
   void resetTagThreads(TAG_TYPE type) { m_threads.resetTag(type); }
   bool getGenReport() const;
   void setGenReport(bool set);
   void setAffinity(int core) const;
   void setPath(const char_t* path) { if(path) m_path.set(path); }
   const STRING& getPath() const { return m_path; }
   void setCrashWatch(WATCH_ITEM* watch_item) { m_crash_watch = watch_item; }
   virtual WATCH_ITEM* getCrashWatch() const { return m_crash_watch; }
	PROCESS() : EXECOBJ(EXECOBJ_PROCESS, 0) {Init();}//used by PROCESS_LIST for list key
	virtual ~PROCESS();
private:
   unsigned long cpuTime(bool avg);
   unsigned long totalTimeDiff(bool avg);
	PROCESS(EXECOBJ_DATA *data, DEVICE *device);
	void Init();
	
	THREAD_LIST m_threads;
	EXECOBJ_DATA m_data[3];
	WATCH_ITEM *m_crash_watch;
	unsigned long m_mem[3];
   STRING m_path;
};

#endif//PROCESS_H