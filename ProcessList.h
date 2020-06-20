#ifndef PROCESS_LIST_H
#define PROCESS_LIST_H

#include "../ds/ds.h"

enum SORT_BY
{
   SORT_BY_NAME			= 2,
   SORT_BY_NAME_R		= SORT_BY_NAME | 1,
   SORT_BY_ID			= SORT_BY_NAME << 1,
   SORT_BY_ID_R			= SORT_BY_ID | 1,
   SORT_BY_CPU			= SORT_BY_ID << 1,
   SORT_BY_CPU_R		= SORT_BY_CPU | 1,
   SORT_BY_CORE			= SORT_BY_CPU << 1,
   SORT_BY_CORE_R		= SORT_BY_CORE | 1,
   SORT_BY_CPU_AVG		= SORT_BY_CORE << 1,
   SORT_BY_CPU_AVG_R	= SORT_BY_CPU_AVG | 1,
   SORT_BY_CPU_MAX		= SORT_BY_CPU_AVG << 1,
   SORT_BY_CPU_MAX_R	= SORT_BY_CPU_MAX | 1,
   SORT_BY_MEM			= SORT_BY_CPU_MAX << 1,
   SORT_BY_MEM_R		= SORT_BY_MEM | 1,
   SORT_BY_MEM_MAX		= SORT_BY_MEM << 1,
   SORT_BY_MEM_MAX_R	= SORT_BY_MEM_MAX | 1
};

class DEVICE;
class PROCESS_LIST : public SORTED_LIST
{
   DEVICE* m_device;
public:
	virtual int Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const;
   PROCESS* getProcessById(DWORD id) const;
   void setDevice(DEVICE *device) { m_device = device; }
   DEVICE * getDevice() { return m_device; }
   PROCESS* addProcess(EXECOBJ_DATA *data, DEVICE* device);
   THREAD* addThread(EXECOBJ_DATA *data);
   void setSortTechnique(SORT_BY sortTechnique);
   void setIsAlive(char alive);
   void resetTag(TAG_TYPE type);
   void removeUntaggedProcesses();
   int GetThreadCount() const;
};

class DISPLAY_LIST : public SORTED_LIST
{
   SORT_BY m_sortBy;
   bool m_showThreads;
   int compareByName(EXECOBJ *d1, EXECOBJ *d2) const;
   int compareByCPU(EXECOBJ *d1, EXECOBJ *d2, bool avg) const;
   int compareByMem(EXECOBJ *d1, EXECOBJ *d2) const;
   int compareByCore(EXECOBJ *d1, EXECOBJ *d2) const;
   int compareById(EXECOBJ *d1, EXECOBJ *d2) const;
   int compareByMaxCpu(EXECOBJ *d1, EXECOBJ *d2) const;
   int compareByMaxMem(EXECOBJ *d1, EXECOBJ *d2) const;
public:
   DISPLAY_LIST();
   void setSortTechnique(SORT_BY sortTechnique);
   int getSortTechnique();
   bool getShowThreads();
   void toggleShowThreads();
   void setShowThreads(bool show);
   void copyMembers(const DISPLAY_LIST &dl);
   virtual int Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const;
   int Insert(PROCESS* p);
   void Destroy();
};

#endif//PROCESS_LIST_H