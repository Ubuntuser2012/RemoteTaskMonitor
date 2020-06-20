#ifndef DEVICE_H
#define DEVICE_H
// Device.cpp : implementation file
//

#include "stdafx.h"

class HISTORY;
class WATCH_ITEM;
class ADDR_INFO;

class DEVICE : public OBJECT
{
public:
	static DEVICE* create(unsigned long ip, unsigned short port, SOCKET so);
	int readSocket(STRING &err);
	int consumeStreamData();
	unsigned long getIp() const;
	unsigned short getPort() const;
	int readFile(STRING &file, HISTORY &history, STRING &err);
	int sendNoop();
	int sendGetThreadNames();
	int sendKillProcess(PROCESS* p);
	int sendStartProcess(const char_t *path);
	int sendCloneProcess(PROCESS* p);
	int sendRestartProcess(PROCESS* p);
	int sendKillThread(THREAD* t);
	int sendSuspendThread(THREAD* t);
	int sendResumeThread(THREAD* t);
	int sendSetThreadPriority(int tid, int priority);
	int sendResetDevice();
	int sendResetServer();
	int sendCaptureOnDev(DWORD priority, BOOL capture_on_dev, DWORD interval_ms);
	int sendSuspend();
	int sendShutdown();
	int sendDisconnect();
	int sendGetSnapInfo();
	int sendGetMemoryInfo(bool show);
	int sendGetProcessPath(DWORD pid);
	int sendSetProcessAffinity(DWORD pid, int affinity);
	int sendSetThreadAffinity(DWORD tid, int affinity);
	void removeWatch(WATCH_ITEM *watch) { if(watch) m_watchlist.Remove(watch); }
	void cancelPsWatches();
	void checkCPUWatch(float cpuPercent);
	int getNumProcesses() const { return m_proceses.GetCount(); }
	int getNumThreads() const { return m_proceses.GetThreadCount(); }
	PROCESS* getProcess(int i) const;
	PROCESS* removeProcess(int i);
	PROCESS* removeProcess(PROCESS* p);
	void debugProcess(WATCH_ITEM* watch_item);
	PROCESS* getProcessById(int id)  const;
	PROCESS* addProcess(EXECOBJ_DATA *data, DEVICE* device) { return m_proceses.addProcess(data, device); }
	THREAD* addThread(EXECOBJ_DATA *data) { return m_proceses.addThread(data); }
	void removeDeadProcesses();
	MY_SYSTEM_INFO* getSysInfo() { return &m_sysinfo; }
	const MY_MEM_INFO *getMemStatus() const;
	float memPercent() const;
	STRING totalMem() const;
	STRING freeMem() const;
	STRING processArchitecture() const;
	STRING clockSpeed() const;
	float powerPercent() const;
	BOOL powerSourceIsAC() const;
	DWORD powerSourceIsBattery() const;
	WATCH_ITEM *getMemWatch() { return m_mem_watch; }
	WATCH_ITEM *getCPUWatch() { return m_cpu_watch; }
	void delTotalCPUWatch();
	void delMemWatch();
	void delCPUWatch();
	void setMemWatch(WATCH_ITEM *watch_item) { m_mem_watch = watch_item; }
	void setCPUWatch(WATCH_ITEM *watch_item) { m_cpu_watch = watch_item; }
	int getProcessCount(bool includeThreads) const;
	void getTimeStamp(SYSTEMTIME& ts) const;
	void close();
	void resetTag(TAG_TYPE type) { m_proceses.resetTag(type); }
	void setThreadName(DWORD tid, char_t name[64]);
	void addSnapInfo(COMM_SNAPINFO *data);
	int checkVersion(int server_version, int client_version, STRING &err);
	COMM_INIT_SETTINGS *getInitSettings() { return &m_init_settings; }
	float cpuPercentBySummingCoreCpu(bool avg, unsigned long core) const;
	float cpuPercent(bool avg) const;
	int numCores() const;

	virtual ~DEVICE();

private:
	DEVICE(unsigned long ip, unsigned short port, SOCKET so);

	void gotProcessPath(DWORD pid, const char_t *name);
	int sendDebugNewProcess(DWORD rqstid, const char_t *path);
	int sendDebugActiveProcess(DWORD rqstid, DWORD pid);
	void debugProcessAckRecvd(DEBUG_PROCESS_ACK *ack);
	void debugProcessCrashInfoRecvd(CRASH_INFO *cinfo);
	void checkMemWatch(DWORD available_mem);
	void watchTrigger(bool condition, WATCH_ITEM *watch, CDialog *dlg);
	long64 cpuIdleTime(bool avg, int core)  const;
	long64 totalTimeDiff(bool avg)  const;
	long64 getTimeStamp(int index) const;
	int parseMessage(COMM_MESG *msg, STRING &err);
	float cpuPercentBySummingCoreCpu(bool avg) const;
	float cpuPercentBySummingProcessCpu(bool avg) const;

	mutable float m_max_cpu[MAX_CORES];
	SOCKET m_so;
	COMM_SNAPINFO m_snapinfo[3];
	PROCESS_LIST m_proceses;
	WATCH_LIST m_watchlist;
	WATCH_ITEM *m_mem_watch;
	WATCH_ITEM *m_cpu_watch;
	COMM_INIT_SETTINGS m_init_settings;
	MY_SYSTEM_INFO m_sysinfo;
	char *m_stream;
	int m_streamSize;
	int m_streamHead;
	int m_streamTail;
	char m_index;
	unsigned long m_ip;
	unsigned short m_port;
};

#endif//DEVICE_H