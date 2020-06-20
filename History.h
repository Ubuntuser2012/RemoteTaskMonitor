#ifndef HISTORY_H
#define HISTORY_H
// 
//

#include "stdafx.h"
#include "datapoint.h"

class DEVICE;
class GDGraph;
class CDlgRemoteCPU;
class PROCESS;
class THREAD;

class DATA_VALUE : public OBJECT
{
public:
   union
   {
      DWORD dw;
      float fl;
      SYSTEMTIME ts;
   }u;
   DATA_VALUE(DWORD _dw) : OBJECT() { u.dw = _dw; }
   DATA_VALUE(float _fl) : OBJECT() { u.fl = _fl; }
   DATA_VALUE(SYSTEMTIME &_ts) : OBJECT() { memcpy(&u.ts, &_ts, sizeof(SYSTEMTIME)); }
   virtual ~DATA_VALUE() {}
};

class DATA_QUE : public QUE
{
   STRING m_str;
   STRING m_outdir;
   STRING m_file;
   FILE *m_fp;
   bool m_first;

   int readf();

public:
	DATA_QUE(int max, STRING &out_dir);
	int Init(int max, STRING &out_dir);

	virtual ~DATA_QUE();

	int Enque(OBJECT* p);
	int Enque(DWORD _dw);
	int Enque(float _fl);
	int Enque(SYSTEMTIME &_ts);
	
	STRING &labels();
	STRING &cpu();
	STRING &mem();
};



class DATA_LIST
{
	DATA_POINT m_data[MAX_DATA_LIST_SIZE];
	DWORD m_pid;
	DWORD m_tid;
   STRING m_outdir;
   FILE *m_fp;
   GDGraph &m_gd;
	char_t m_fileprefix[32];
	int m_reset_count;
	int m_count;

public:
   DATA_LIST::DATA_LIST(GDGraph &gd, const char_t* outdir, const char_t* fileprefix, DWORD pid, DWORD tid);

	virtual ~DATA_LIST();

	bool Insert(float val, SYSTEMTIME &ts);
   void SetDir(STRING &outdir);
   int rc() const { return m_reset_count; }
   DWORD pid() const { return m_pid; }
   DWORD tid() const { return m_tid; }
	bool Flush(bool close);
};

class HISTORY_DATA : public OBJECT
{
public:
	DWORD m_pid;
	DWORD m_tid;
   unsigned char m_priority;
   GDGraph &m_gd;
   char_t m_pname[64];
   char_t m_tname[64];
   DATA_LIST m_cpu;
   DATA_LIST m_mem;

   HISTORY_DATA(GDGraph &gd, STRING &out_dir, DWORD pid, const char2_t *pname, DWORD tid, char_t *tname, unsigned char priority);
   virtual ~HISTORY_DATA() {}
};

enum {
   HISTORY_STATE_STOPPED,
   HISTORY_STATE_STARTED,
};

enum {
	HISTORY_FILE_PS,
	HISTORY_FILE_TH,
	HISTORY_FILE_MEM,
	HISTORY_NUM_FILES
};
class GDGraph;
class HISTORY
{
public:
   HISTORY(CDlgRemoteCPU *parent);
   virtual ~HISTORY();
   bool Start(const char_t *filename=0);
   bool Started() const { return m_state==HISTORY_STATE_STARTED; }
   bool Stop(DEVICE *device, STRING &outdir, CDlgRemoteCPU* parent);
   bool Write(DEVICE *device, CDlgRemoteCPU* parent);
   STRING& GetDirectory();
   void SetDirectory(STRING& dir);

private:
   CDlgRemoteCPU *m_parent;
   LIST m_history_tags[2];
   STRING m_history_fname[HISTORY_NUM_FILES];
   STRING m_outdir;
   DWORD m_num_writes;
   int m_state;
   DATA_LIST m_cpu;
   DATA_LIST m_mem;
   GDGraph m_gd;
   STRING m_dir;

   bool Tags();
   bool WriteTempData(HISTORY_DATA *h, PROCESS* p, THREAD* t, SYSTEMTIME &ts, const MY_MEM_INFO *mem);
   bool WriteTempData(DWORD memoryLoad, float cpu, SYSTEMTIME &ts);
   bool PrependTempFilesWhiteSpace();
   void GenerateReport(DEVICE *device, CDlgRemoteCPU* parent);
};

#endif//HISTORY_H