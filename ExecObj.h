#ifndef EXECOBJ_H
#define EXECOBJ_H

enum {
   EXECOBJ_THREAD,
   EXECOBJ_PROCESS
};
enum TAG_TYPE {
   TAG_TYPE_HISTORY,
   TAG_TYPE_LIVE_PS,
   TAG_TYPES
};


class WATCH_ITEM;
class DEVICE;

class EXECOBJ :public OBJECT
{
private:
   char m_type;
   char m_checked;
   bool m_tagged[TAG_TYPES];//used by history
   bool m_genReport;//used by history
   WATCH_ITEM *m_watch_item;//used for CPU watch
protected:
   float m_max_cpu;
   mutable DWORD m_max_mem;
   long64 timeDelta(bool avg);
   long64 m_timeStamp[3];
   char m_index;
	DEVICE *m_device;

   float cpuPercent1(DEVICE* device, long64 timed, long64 cput);
   void getTimeStamp(SYSTEMTIME& ts);
public:
   EXECOBJ(char type, DEVICE *device) : OBJECT(), m_type(type), m_checked(0), m_genReport(false), m_max_cpu(0), m_max_mem(0), m_watch_item(0), m_device(device) { memset(m_tagged, 0, sizeof(m_tagged));}
   virtual ~EXECOBJ();
   void Init();
   WATCH_ITEM *getCPUWatch() { return m_watch_item; }
   void delCPUWatch();
   void setCPUWatch(WATCH_ITEM *watch_item) { m_watch_item = watch_item; }
   char getType() { return m_type; }
   char getChecked() { return m_checked; }
   void setChecked(char state) { m_checked=state; }
   char toggleChecked() { m_checked^=1; return m_checked;}
   void resetTag(TAG_TYPE type) { m_tagged[type] = false; }
   void setTag(TAG_TYPE type) { m_tagged[type] = true; }
   bool isTagged(TAG_TYPE type) const { return m_tagged[type]; }
   bool getGenReport() const { return m_genReport; }
   void setGenReport(bool set) { m_genReport = set; }
   virtual void setAffinity(int core) const = 0;
   float getMaxCpu() const { return m_max_cpu; }
   DWORD getMaxMem() const { return m_max_mem; }
	DEVICE* getDevice() const { return m_device; }
   virtual WATCH_ITEM* getCrashWatch() const { return 0; }
   virtual int affinity() const = 0;
};

#endif