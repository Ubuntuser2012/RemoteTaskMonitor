#ifndef GDGRAPH_H
#define GDGRAPH_H

extern "C" 
{
#include <gd.h>
#include <gdfontg.h>
#include <gdfontl.h>
}
#include "datapoint.h"

class DATA_LIST;
class CDlgRemoteCPU;
class CDialog2;
class DEVICE;
class PROCESS;

class WORK_ITEM : public OBJECT
{
public:
	DATA_POINT m_data[MAX_DATA_LIST_SIZE];
	int m_num_samples;
	STRING m_file;
};

class GDGraph
{
   gdImagePtr gdImageTrueColorAttachBuffer(int* buffer, int sx, int sy, int stride);
   void gdImageDetachBuffer(gdImagePtr im);
   BITMAPINFO gdCreateBmp(int width, int height);
   static DWORD WINAPI gdThreadProc(LPVOID lpParameter);
   bool gdPlotGraph(STRING &file, DATA_POINT_ *data, int data_sz);
   void gdDeque();

   HANDLE m_thread[4];
   HANDLE m_sem;
   CRITICAL_SECTION m_qlock;
   LIST m_que;
   bool m_quit;
   int m_thread_count;

public:
   GDGraph();
   virtual ~GDGraph();

   bool gdEnque(STRING &file, DATA_POINT *data, int data_sz);
	bool gdReport(CDialog2 &dlg,
					   DEVICE * device, 
					   STRING &outdir, 
					   CDlgRemoteCPU* parent,
					   LIST *history_tags,
					   DATA_LIST &cpu,
					   DATA_LIST &mem
	);
   int gdRemaining();
   void gdStop();
   void gdStart();
};

#endif//GDGRAPH_H