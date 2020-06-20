#ifndef WATCH_LIST_H
#define WATCH_LIST_H

class DEVICE;
class CDlgWatchMemAlert;
class CDlgWatchCPUAlert;
class CDlgWatchCPUTotalAlert;

class WATCH_LIST : public SORTED_LIST
{
	DEVICE* m_device;
public:
	virtual int Compare(const OBJECT* visited_obj, const OBJECT* user_obj) const;
	void setDevice(DEVICE *device) { m_device = device; }
	DEVICE * getDevice() { return m_device; }
	WATCH_ITEM* getItem(int id);
};

class WATCH_ITEM : public OBJECT
{
	WATCH_ITEM(
		bool debug_new_ps, 
		DWORD pid, 
		const char_t *pspath, 
		const char_t *mapfile, 
		const char_t *emailto,
		const char_t *pspath2, 
		bool watch_ps2, 
		bool reset_device, 
		bool silent_mode);
	
	WATCH_ITEM(
		int min_mem,
		DEVICE *device, 
		int duration,
		const char_t *emailto,
		bool reset_device);

   WATCH_ITEM(
      float max_cpu,
		DEVICE *device, 
		DWORD pid,
		DWORD tid,
      int duration,
      const char_t *emailto,
      bool reset_device,
      bool kill_ps,
      bool restart_ps);

   WATCH_ITEM(
      float max_cpu,
		DEVICE *device, 
      int duration,
      const char_t *emailto,
      bool reset_device);

	CDlgWatchMemAlert *m_dlgMemWatchAlert;
	CDlgWatchCPUAlert *m_dlgCPUWatchAlert;
	CDlgWatchCPUTotalAlert *m_dlgTotalCPUWatchAlert;
   DEVICE *m_device;

public:
   virtual ~WATCH_ITEM();

   static WATCH_ITEM* create(
		bool debug_new_ps, 
		DWORD pid, 
		const char_t *pspath, 
		const char_t *mapfile, 
		const char_t *emailto,
		const char_t *pspath2, 
		bool watch_ps2, 
		bool reset_device, 
		bool silent_mode);

	static WATCH_ITEM* create(
		int min_mem,
		DEVICE *device, 
		int duration,
		const char_t *emailto,
		bool reset_device);

   static WATCH_ITEM* create(
      float max_cpu,
		DEVICE *device, 
		DWORD pid,
		DWORD tid,
      int duration,
      const char_t *emailto,
      bool reset_device,
      bool kill_ps,
      bool restart_ps);

   static WATCH_ITEM* create(
      float max_cpu,
		DEVICE *device, 
      int duration,
      const char_t *emailto,
      bool reset_device);

   void set(
      int min_mem,
	  DEVICE *device,
      int duration,
      const char_t *emailto,
      bool reset_device);

   void set(
      float max_cpu,
		DEVICE *device, 
		DWORD pid,
		DWORD tid,
      int duration,
      const char_t *emailto,
      bool reset_device,
      bool kill_ps,
      bool restart_ps);

   void set(
      float max_cpu,
		DEVICE *device, 
      int duration,
      const char_t *emailto,
      bool reset_device);

   bool m_debug_new_ps;
	DWORD m_pid;
	DWORD m_tid;
	STRING m_pspath;
	STRING m_mapfile;
	STRING m_emailto;

	STRING m_pspath2;
	bool m_watch_ps2;
	bool m_reset_device;
	bool m_silent_mode;

	DWORD m_min_mem;
	DWORD m_duration;
   DWORD m_started_at;
	
   float m_max_cpu;
   bool m_kill_ps;
   bool m_restart_ps;



   void trigger(bool condition, DEVICE *device);
};

#endif//WATCH_LIST_H