#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "Process.h"
#include "ProcessList.h"
#include "watchlist.h"
#include "device.h"

//////////////////////////////////////////////////

EXECOBJ::~EXECOBJ()
{ 
   if(m_watch_item) 
      delete m_watch_item; 
}

void EXECOBJ::Init()
{
   m_index = 0;
}

void EXECOBJ::delCPUWatch() 
{ 
   if(m_watch_item) 
      delete m_watch_item; 
   m_watch_item = 0; 
}
