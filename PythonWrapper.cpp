// pytest.cpp : Defines the entry point for the console application.
//

//parse tuple: http://docs.python.org/release/1.5.2p2/ext/parseTuple.html
//Extending Python with C or C++: http://docs.python.org/extending/extending.html
//Use http://pygments.org for syntax high lighting

#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "resource.h"
#include "RemoteCPU.h"
#include "DlgRemoteCPU.h"
#include "DlgWatchCfg.h"
#include "DlgWatchCPU.h"
#include "DlgWatchCPUTotal.h"
#include "WatchList.h"

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif//_DEBUG

enum PY_NOTIFICATION
{
   PY_NOTIFICATION_DISCONNECTED=0,
   PY_NOTIFICATION_CONNECTED,
   PY_NOTIFICATION_GOT_PS_PATH,
   PY_NOTIFICATION_GOT_SNAPSHOT,
   PY_NOTIFICATION_CRASHWATCH_ACK,
   PY_NOTIFICATION_CRASHWATCH_NACK,
   PY_NOTIFICATION_EXCEPTION,
   PY_NOTIFICATION_MEM_ALERT,
   PY_NOTIFICATION_TOTAL_CPU_ALERT,
   PY_NOTIFICATION_PS_CPU_ALERT,
   PY_NOTIFICATION_TH_CPU_ALERT,
};

class PY_NOTIFY : public OBJECT
{
public:
   PY_NOTIFY(int state, DWORD pid, DWORD tid, const char_t* path, const char_t* msg)
      : m_state(0), m_pid(0), m_tid(0)
   {
      m_state = state;
      m_pid = pid;
      m_tid = tid;
      if(msg)
         m_msg.set(msg);
      if(path)
         m_path.set(path);
   }

   int m_state;
   DWORD m_pid;
   DWORD m_tid;
   STRING m_msg;
   STRING m_path;
};

static bool py_initialized;
static int py_con_state;
static QUE rtm_q;
static PyObject *PyModule = 0;
static int rtm_call(PyObject *pModule, char* func, int argc, const char** argv);

//Functions that py script can call
static PyObject* rtm_wait(PyObject *self, PyObject *args);
static PyObject* rtm_connect(PyObject *self, PyObject *args);
static PyObject* rtm_isconnected(PyObject *self, PyObject *args);
static PyObject* rtm_disconnect(PyObject *self, PyObject *args);
static PyObject* rtm_startcapture(PyObject *self, PyObject *args);
static PyObject* rtm_stopcapture(PyObject *self, PyObject *args);
static PyObject* rtm_getcaptureinterval(PyObject *self, PyObject *args);
static PyObject* rtm_setcaptureinterval(PyObject *self, PyObject *args);
static PyObject* rtm_getsmtp(PyObject *self, PyObject *args);
static PyObject* rtm_setsmtp(PyObject *self, PyObject *args);
static PyObject* rtm_enablepsmemstats(PyObject *self, PyObject *args);
static PyObject* rtm_saveondev(PyObject *self, PyObject *args);
static PyObject* rtm_resetdevice(PyObject *self, PyObject *args);
static PyObject* rtm_getnumprocessors(PyObject *self, PyObject *args);
static PyObject* rtm_gettotalmem(PyObject *self, PyObject *args);
static PyObject* rtm_getavailmem(PyObject *self, PyObject *args);
static PyObject* rtm_gettotalcpu(PyObject *self, PyObject *args);
static PyObject* rtm_restartcemon(PyObject *self, PyObject *args);
static PyObject* rtm_killps(PyObject *self, PyObject *args);
static PyObject* rtm_startps(PyObject *self, PyObject *args);
static PyObject* rtm_cloneps(PyObject *self, PyObject *args);
static PyObject* rtm_restartps(PyObject *self, PyObject *args);
static PyObject* rtm_getnumps(PyObject *self, PyObject *args);
static PyObject* rtm_getpss(PyObject *self, PyObject *args);
static PyObject* rtm_getpsids(PyObject *self, PyObject *args);
static PyObject* rtm_getpsname(PyObject *self, PyObject *args);
static PyObject* rtm_getpspath(PyObject *self, PyObject *args);
static PyObject* rtm_getpscpu(PyObject *self, PyObject *args);
static PyObject* rtm_getpsavgcpu(PyObject *self, PyObject *args);
static PyObject* rtm_getpsmaxcpu(PyObject *self, PyObject *args);
static PyObject* rtm_getpsmemused(PyObject *self, PyObject *args);
static PyObject* rtm_getpsnumthreads(PyObject *self, PyObject *args);
static PyObject* rtm_getthreads(PyObject *self, PyObject *args);
static PyObject* rtm_getthreadids(PyObject *self, PyObject *args);
static PyObject* rtm_getthreadname(PyObject *self, PyObject *args);
static PyObject* rtm_setthreadname(PyObject *self, PyObject *args);
static PyObject* rtm_getthreadcpu(PyObject *self, PyObject *args);
static PyObject* rtm_getthreadavgcpu(PyObject *self, PyObject *args);
static PyObject* rtm_getthreadmaxcpu(PyObject *self, PyObject *args);
static PyObject* rtm_addpstoreport(PyObject *self, PyObject *args);
static PyObject* rtm_removepsfromreport(PyObject *self, PyObject *args);
static PyObject* rtm_addthtoreport(PyObject *self, PyObject *args);
static PyObject* rtm_removethfromreport(PyObject *self, PyObject *args);
static PyObject* rtm_getthreadpriority(PyObject *self, PyObject *args);
static PyObject* rtm_setthreadpriority(PyObject *self, PyObject *args);
static PyObject* rtm_crashwatchps(PyObject *self, PyObject *args);
static PyObject* rtm_crashwatchnewps(PyObject *self, PyObject *args);
static PyObject* rtm_cpuwatch(PyObject *self, PyObject *args);
static PyObject* rtm_cpuwatchcancel(PyObject *self, PyObject *args);
static PyObject* rtm_cpuwatchps(PyObject *self, PyObject *args);
static PyObject* rtm_cpuwatchpscancel(PyObject *self, PyObject *args);
static PyObject* rtm_cpuwatchth(PyObject *self, PyObject *args);
static PyObject* rtm_cpuwatchthcancel(PyObject *self, PyObject *args);
static PyObject* rtm_memwatch(PyObject *self, PyObject *args);
static PyObject* rtm_memwatchcancel(PyObject *self, PyObject *args);
static PyObject* rtm_email(PyObject *self, PyObject *args);
static PyObject* rtm_write(PyObject *self, PyObject *args);
static PyObject* rtm_flush(PyObject *self, PyObject *args);

//Functions that c code will call int py script
//ondisconnect(report_file)
//oncrash(pid, msg)
//oncpuwatch(msg)
//oncpuwatchps(pid, msg)
//oncpuwatchth(tid, msg)
//onmemwatch(msg)

static PyMethodDef RtmMethods[] = {
   {"wait", rtm_wait, METH_VARARGS, "rtm.wait()"},
	{"connect", rtm_connect, METH_VARARGS, "rtm.connect(ip, 45)"},
	{"isconnected", rtm_isconnected, METH_VARARGS, "rtm.isconnected()"},
	{"disconnect", rtm_disconnect, METH_VARARGS, "rtm.disconnect()"},
	{"startcapture", rtm_startcapture, METH_VARARGS, "rtm.startcapture()"},
	{"stopcapture", rtm_stopcapture, METH_VARARGS, "rtm.stopcapture()"},
	{"getcaptureinterval", rtm_getcaptureinterval, METH_VARARGS, "rtm.getcaptureinterval()"},
	{"setcaptureinterval", rtm_setcaptureinterval, METH_VARARGS, "rtm.setcaptureinterval(int)"},
	{"getsmtp", rtm_getsmtp, METH_VARARGS, "rtm.getsmtp()"},
	{"setsmtp", rtm_setsmtp, METH_VARARGS, "rtm.setsmtp(ip, 25)"},
	{"enablepsmemstats", rtm_enablepsmemstats, METH_VARARGS, "rtm.enablepsmemstats(1|0)"},
	{"saveondev", rtm_saveondev, METH_VARARGS, "rtm.saveondev(1|0)"},
	{"resetdevice", rtm_resetdevice, METH_VARARGS, "rtm.resetdevice()"},
	{"getnumprocessors", rtm_getnumprocessors, METH_VARARGS, "rtm.getnumprocessors()"},
	{"gettotalmem", rtm_gettotalmem, METH_VARARGS, "rtm.gettotalmem()"},
	{"getavailmem", rtm_getavailmem, METH_VARARGS, "rtm.getavailmem()"},
	{"gettotalcpu", rtm_gettotalcpu, METH_VARARGS, "rtm.gettotalcpu()"},
	{"restartcemon", rtm_restartcemon, METH_VARARGS, "rtm.restartcemon()"},
	{"killps", rtm_killps, METH_VARARGS, "rtm.killps(pid)"},
	{"startps", rtm_startps, METH_VARARGS, "rtm.startps(path)"},
	{"cloneps", rtm_cloneps, METH_VARARGS, "rtm.cloneps(pid)"},
	{"restartps", rtm_restartps, METH_VARARGS, "rtm.restartps(pid)"},
	{"getnumps", rtm_getnumps, METH_VARARGS, "rtm.getnumps()"},
	{"getpss", rtm_getpss, METH_VARARGS, "rtm.getpss()"},
	{"getpsids", rtm_getpsids, METH_VARARGS, "rtm.getpsids()"},
	{"getpsname", rtm_getpsname, METH_VARARGS, "rtm.getpsname(pid)"},
	{"getpspath", rtm_getpspath, METH_VARARGS, "rtm.getpspath(pid)"},
	{"getpscpu", rtm_getpscpu, METH_VARARGS, "rtm.getpscpu(pid)"},
	{"getpsavgcpu", rtm_getpsavgcpu, METH_VARARGS, "rtm.getpsavgcpu(pid)"},
	{"getpsmaxcpu", rtm_getpsmaxcpu, METH_VARARGS, "rtm.getpsmaxcpu(pid)"},
	{"getpsmemused", rtm_getpsmemused, METH_VARARGS, "rtm.getpsmemused(pid)"},
	{"getpsnumthreads", rtm_getpsnumthreads, METH_VARARGS, "rtm.getpsnumthreads(pid)"},
	{"getthreads", rtm_getthreads, METH_VARARGS, "rtm.getthreads(pid)"},
	{"getthreadids", rtm_getthreadids, METH_VARARGS, "rtm.getthreadids(pid)"},
	{"getthreadname", rtm_getthreadname, METH_VARARGS, "rtm.getthreadname(pid, tid)"},
	{"setthreadname", rtm_setthreadname, METH_VARARGS, "rtm.setthreadname(pid, tid, tname)"},
	{"getthreadcpu", rtm_getthreadcpu, METH_VARARGS, "rtm.getthreadcpu(pid, tid)"},
	{"getthreadavgcpu", rtm_getthreadavgcpu, METH_VARARGS, "rtm.getthreadavgcpu(pid, tid)"},
	{"getthreadmaxcpu", rtm_getthreadmaxcpu, METH_VARARGS, "rtm.getthreadmaxcpu(pid, tid)"},
	{"addpstoreport", rtm_addpstoreport, METH_VARARGS, "rtm.addpstoreport(pid)"},
	{"removepsfromreport", rtm_removepsfromreport, METH_VARARGS, "rtm.removepsfromreport(pid)"},
	{"addthtoreport", rtm_addthtoreport, METH_VARARGS, "rtm.addthtoreport(pid, tid)"},
	{"removethfromreport", rtm_removethfromreport, METH_VARARGS, "rtm.removethfromreport(pid, tid)"},
	{"getthreadpriority", rtm_getthreadpriority, METH_VARARGS, "rtm.getthreadpriority(pid, tid)"},
	{"setthreadpriority", rtm_setthreadpriority, METH_VARARGS, "rtm.setthreadpriority(pid, tid, priority)"},
	{"crashwatchps", rtm_crashwatchps, METH_VARARGS, "rtm.crashwatchps(pid, mapfile)"},
	{"crashwatchnewps", rtm_crashwatchnewps, METH_VARARGS, "rtm.crashwatchnewps(pspath, mapfile)"},
	{"cpuwatch", rtm_cpuwatch, METH_VARARGS, "rtm.cpuwatch(more_than_cpu_percent, for_so_many_seconds)"},
	{"cpuwatchcancel", rtm_cpuwatchcancel, METH_VARARGS, "rtm.cpuwatchcancel()"},
	{"cpuwatchps", rtm_cpuwatchps, METH_VARARGS, "rtm.cpuwatchps(pid, more_than_cpu_percent, for_so_many_seconds)"},
	{"cpuwatchpscancel", rtm_cpuwatchpscancel, METH_VARARGS, "rtm.cpuwatchpscancel(pid)"},
	{"cpuwatchth", rtm_cpuwatchth, METH_VARARGS, "rtm.cpuwatchth(pid, tid, more_than_cpu_percent, for_so_many_seconds)"},
	{"cpuwatchthcancel", rtm_cpuwatchthcancel, METH_VARARGS, "rtm.cpuwatchthcancel(pid, tid)"},
	{"memwatch", rtm_memwatch, METH_VARARGS, "rtm.memwatch(less_than_bytes, for_so_many_seconds)"},
	{"memwatchcancel", rtm_memwatchcancel, METH_VARARGS, "rtm.memwatchcancel()"},
	{"email", rtm_email, METH_VARARGS, "rtm.email(to, subject, msg)"},
   {"write", rtm_write, METH_VARARGS, "rtm.write(msg)"},
   {"flush", rtm_flush, METH_VARARGS, "rtm.flush(msg)"},
	{NULL, NULL, 0, NULL}
};

static PyModuleDef RtmModule = {
	PyModuleDef_HEAD_INIT, "rtm", NULL, -1, RtmMethods,
	NULL, NULL, NULL, NULL
};

static bool
py_connected_or_connecting()
{
   return (py_con_state == CONNECTION_DATA_STATE__CONNECTED || py_con_state == CONNECTION_DATA_STATE__CONNECTING);
}

static PyObject*
PyInit_rtm(void)
{
	return PyModule_Create(&RtmModule);
}

int atoi_s(const char *_Str)
{
	STRING tmp;
	tmp.set(_Str);
	return atoiW(tmp.s());
}

int rtm_call(PyObject *pModule, char* func, int argc, const char** argv)
{
	int ret = 0;
	PyObject *pArgs;
	PyObject *pValue;
	PyObject *pFunc = PyObject_GetAttrString(pModule, func);

	if (!pFunc || !PyCallable_Check(pFunc))
	{
		if (PyErr_Occurred())
			PyErr_Print();
		fprintf(stderr, "Cannot find function \"%s\"\n", func);
		goto Exit;
	}
	else
	{
		pArgs = PyTuple_New(argc);
		for (int i = 0; i < argc; ++i) 
		{
			if(0 != atoi_s(argv[i]))
				pValue = PyLong_FromLong(atoi_s(argv[i]));
			else
				pValue = PyUnicode_FromString(argv[i]);
			if (!pValue) 
			{
				Py_DECREF(pArgs);
				fprintf(stderr, "Cannot convert argument\n");
				goto Exit;
			}
			// pValue reference stolen here:
			PyTuple_SetItem(pArgs, i, pValue);
		}
		pValue = PyObject_CallObject(pFunc, pArgs);
		Py_DECREF(pArgs);
		if (!pValue) 
		{
			Py_DECREF(pFunc);
			PyErr_Print();
			fprintf(stderr,"Call failed\n");
			goto Exit;
		}
		else
		{
			//printf("Result of call: %ld\n", PyLong_AsLong(pValue));
			Py_DECREF(pValue);
		}
	}

	if(pFunc)
	{
		Py_XDECREF(pFunc);
	}
	ret = 1;
Exit:
	return ret;
}

static PyObject* rtm_wait(PyObject *self, PyObject *args)
{
   //returns: "connected", "disconnected", "timedout", "quit", "gotsnapshot"
   PyObject *ret = NULL;
   unsigned int timeout = -1;
   int start = GetTickCount();

	PyArg_ParseTuple(args, "i", &timeout);

	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();

   PY_NOTIFY *notify = 0;
   bool brk = false;
	while(!brk)
	{
		if(0 == mainWnd->ProcessWinMessages(mainWnd))//busy wait
      {
         ret = Py_BuildValue("{s:i}", "reason", "quit");
         brk = true;
		}
      else if((timeout != -1) && (GetTickCount() - start > timeout))
      {
         ret = Py_BuildValue("{s:i}", "reason", "timedout");
         brk = true;
      }
      else if(notify = (PY_NOTIFY *)rtm_q.Deque())
      {
         switch(notify->m_state)
         {
         case PY_NOTIFICATION_DISCONNECTED:
            ret = Py_BuildValue("{s:s,s:s}", "reason", "disconnected", "report", notify->m_path.ansi());
            break;
         case PY_NOTIFICATION_CONNECTED:
            ret = Py_BuildValue("{s:s}", "reason", "connected");
            break;
         case PY_NOTIFICATION_GOT_PS_PATH:
            ret = Py_BuildValue("{s:s,s:s}", "reason", "pspath", "pspath", notify->m_path.ansi());
            break;
         case PY_NOTIFICATION_GOT_SNAPSHOT:
           ret = Py_BuildValue("{s:s}", "reason", "gotsnapshot");
           break;
         case PY_NOTIFICATION_CRASHWATCH_ACK:
            ret = Py_BuildValue("{s:s,s:i,s:i}", "reason", "crashwatchack", "pid", notify->m_pid, "pspath", notify->m_path.ansi());
            break;
         case PY_NOTIFICATION_CRASHWATCH_NACK:
            ret = Py_BuildValue("{s:s,s:i,s:i}", "reason", "crashwatchnack", "pid", notify->m_pid, "pspath", notify->m_path.ansi());
            break;
         case PY_NOTIFICATION_EXCEPTION:
            ret = Py_BuildValue("{s:s,s:i,s:i,s:s}", "reason", "exception", "pid", notify->m_pid, "pspath", notify->m_path.ansi(), "callstack", notify->m_msg.ansi());
            break;
         case PY_NOTIFICATION_MEM_ALERT:
            ret = Py_BuildValue("{s:s,s:s}", "reason", "memalert", "msg", notify->m_msg.ansi());
            break;
         case PY_NOTIFICATION_TOTAL_CPU_ALERT:
            ret = Py_BuildValue("{s:s,s:s}", "reason", "totalcpualert", "msg", notify->m_msg.ansi());
            break;
         case PY_NOTIFICATION_PS_CPU_ALERT:
            ret = Py_BuildValue("{s:s,s:i,s:s}", "reason", "pscpualert", "pid", notify->m_pid, "msg", notify->m_msg.ansi());
            break;
         case PY_NOTIFICATION_TH_CPU_ALERT:
            ret = Py_BuildValue("{s:s,s:i,s:i,s:s}", "reason", "thcpualert", "pid", notify->m_pid, "tid", notify->m_tid, "msg", notify->m_msg.ansi());
            break;
         }
         delete notify;
         brk = true;
      }
      else if(!py_connected_or_connecting())
      {
         ret = Py_BuildValue("{s:s}", "reason", "disconnected");
         brk = true;
      }
	}

   return ret;
}

static PyObject* rtm_connect(PyObject *self, PyObject *args)
{
	char *ip = 0;
	int port=0;

	if(!PyArg_ParseTuple(args, "si", &ip, &port))
		return PyLong_FromLong(0);

	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   if(py_connected_or_connecting())
      return PyLong_FromLong(0);

	if(ip && port)
	{
		mainWnd->Connect(inet_addr(ip), port);
	}

	return PyLong_FromLong(1);
}

static PyObject* rtm_isconnected(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   int ret = (py_con_state == CONNECTION_DATA_STATE__CONNECTED);
	return PyLong_FromLong(ret);
}

static PyObject* rtm_disconnect(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   if(!py_connected_or_connecting())
      return PyLong_FromLong(0);

	mainWnd->Disconnect();

	return PyLong_FromLong(1);
}

static PyObject* rtm_startcapture(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	mainWnd->OnButtonCaptureStart();

	return PyLong_FromLong(1);
}

static PyObject* rtm_stopcapture(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	mainWnd->OnButtonCaptureStop();

	return PyLong_FromLong(1);
}

static PyObject* rtm_getcaptureinterval(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	int interval = mainWnd->GetSnapInterval();

	return PyLong_FromLong(interval);
}

static PyObject* rtm_setcaptureinterval(PyObject *self, PyObject *args)
{
	int interval = 0;
	if(!PyArg_ParseTuple(args, "i", &interval))
		return PyLong_FromLong(0);

	STRING err;
	STRING snap_interval;

	snap_interval.sprintf(L"%i", interval);

	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	bool ret = mainWnd->SetSnapInterval(snap_interval, err);

	return PyLong_FromLong(ret);
}

static PyObject* rtm_getsmtp(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   STRING ip;
   short port = 0;
	mainWnd->m_options.GetSMTPServer(ip);
	port = mainWnd->m_options.GetSMTPPort();

   PyObject *obj = Py_BuildValue("{s:i,s:i}", 
      "ip", ip.ansi(), 
      "port", port);//    {'ip': 12.1.3.4, 'port': 25}

	return obj;
}

static PyObject* rtm_setsmtp(PyObject *self, PyObject *args)
{
	char *_ip = 0;
	int _port = 0;
	if(!PyArg_ParseTuple(args, "si", &_ip, &_port))
		return PyLong_FromLong(0);

	STRING ip;
	STRING port;
   ip.set(_ip);
   port.sprintf(TXT("%i"), _port);

	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   mainWnd->m_options.SetSMTPServer(ip.s());
   mainWnd->m_options.SetSMTPPort(port.s());

	return PyLong_FromLong(1);
}

static PyObject* rtm_enablepsmemstats(PyObject *self, PyObject *args)
{
	int val = 0;
	if(!PyArg_ParseTuple(args, "i", &val))
		return PyLong_FromLong(0);

	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	mainWnd->m_processes.SetCheckShowMemory(val==1);

	return PyLong_FromLong(1);
}

static PyObject* rtm_saveondev(PyObject *self, PyObject *args)
{
	int startcapture = 0;
	if(!PyArg_ParseTuple(args, "i", &startcapture))
		return PyLong_FromLong(0);

	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	if(mainWnd->OnCheckCaptureOnDev(startcapture))
	{
		CButton* chkbox = (CButton*)mainWnd->GetFieldWnd(IDC_CHECK_CAPTURE_ON_DEV);
		chkbox->SetCheck(BST_CHECKED == startcapture ?  BST_UNCHECKED:BST_CHECKED);
	}
	return PyLong_FromLong(1);
}

static PyObject* rtm_resetdevice(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	mainWnd->OnButtonResetDevice();

	return PyLong_FromLong(1);
}

static PyObject* rtm_getnumprocessors(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	int ret = mainWnd->GetNumProcessors();

	return PyLong_FromLong(ret);
}

static PyObject* rtm_gettotalmem(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE *device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

	const MY_MEM_INFO* memst = device->getMemStatus();

   return memst ? PyLong_FromLong(memst->totalMemKB * 1024) : PyLong_FromLong(-1);
}

static PyObject* rtm_getavailmem(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE *device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

	const MY_MEM_INFO* memst = device->getMemStatus();

   return memst ? PyLong_FromLong(memst->freeMemKB * 1024) : PyLong_FromLong(-1);
}

static PyObject* rtm_gettotalcpu(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   double ret = mainWnd->GetTotalCpu();
	return PyFloat_FromDouble(ret);
}

static PyObject* rtm_restartcemon(PyObject *self, PyObject *args)
{
	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
	mainWnd->OnButtonResetServer();

	return PyLong_FromLong(1);
}

static PyObject* rtm_killps(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   int ret = device->sendKillProcess(p);

	return PyLong_FromLong(ret);
}

static PyObject* rtm_startps(PyObject *self, PyObject *args)
{
	char *path = 0;
	if(!PyArg_ParseTuple(args, "s", &path))
		return PyLong_FromLong(0);

   if(!path)
      return PyLong_FromLong(0);

   STRING remoteps;
   remoteps.set(path);

	CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);
   
   int ret = device->sendStartProcess(remoteps.s());

	return PyLong_FromLong(ret);
}

static PyObject* rtm_cloneps(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);
   int ret = device->sendCloneProcess(p);

   return PyLong_FromLong(ret ? 1:0);
}

static PyObject* rtm_restartps(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);
   int ret = device->sendRestartProcess(p);

   return PyLong_FromLong(ret ? 1:0);
}

static PyObject* rtm_getnumps(PyObject *self, PyObject *args)
{
   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   int ret = device->getNumProcesses();
	return PyLong_FromLong(ret);
}

static PyObject* rtm_getpss(PyObject *self, PyObject *args)
{
   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   int max = device->getNumProcesses();
   if(max==0)
      return PyLong_FromLong(0);

	PyObject*list = PyList_New(max);
	for(int i=0; i<max; i++)
	{
      PROCESS* p = device->getProcess(i);
      if(p)
      {
         STRING name(L"");
         name.set(p->getName());
		   PyObject *obj = Py_BuildValue("{s:i,s:s,s:f,s:f,s:f,s:i,s:i}", 
            "pid", p->getId(), 
            "name", name.ansi(), 
            "cpu", p->cpuPercent(0), 
            "avg_cpu", p->cpuPercent(1),
            "max_cpu", p->getMaxCpu(), 
            "mem", p->memoryUsed(),
            "numthreads", p->numThreads());//    {'abc': 123, 'def': 456}
         if(obj)
         {
		      PyList_SetItem(list, i, obj);
         }
      }
	}
   return list;
}

static PyObject* rtm_getpsids(PyObject *self, PyObject *args)
{
   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   int max = device->getNumProcesses();
   if(max==0)
      return PyLong_FromLong(0);

	PyObject*list = PyList_New(max);
	for(int i=0; i<max; i++)
	{
      PROCESS* p = device->getProcess(i);
      if(p)
      {
         PyList_SetItem(list, i, PyLong_FromLong( p->getId() ));
      }
	}
	return list;
}

static PyObject* rtm_getpsname(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   STRING name;
   name.set(p->getName() ? p->getName() : L"");

   return PyUnicode_FromString( name.ansi() );
}

static PyObject* rtm_getpspath(PyObject *self, PyObject *args)
{
   STRING path;
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   int ret = device->sendGetProcessPath(pid);
	return PyLong_FromLong(ret);
}

static PyObject* rtm_getpscpu(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   return PyFloat_FromDouble(p->cpuPercent(0));
}

static PyObject* rtm_getpsavgcpu(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   return PyFloat_FromDouble(p->cpuPercent(1));
}

static PyObject* rtm_getpsmaxcpu(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   return PyFloat_FromDouble(p->getMaxCpu());
}

static PyObject* rtm_getpsmemused(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   return PyLong_FromLong(p->memoryUsed());
}

static PyObject* rtm_getpsnumthreads(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   return PyLong_FromLong(p->numThreads());
}

static PyObject* rtm_getthreads(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

	int max = p->numThreads();
   if(max==0)
      return PyLong_FromLong(0);

   PyObject*list = PyList_New(max);
	for(int i=0; i<max; i++)
	{
      THREAD* t = p->getThread(i);
      if(t)
      {
         STRING name(L"");
         name.set(t->getName());
		   PyObject *obj = Py_BuildValue("{s:i,s:s,s:f,s:f,s:f,s:i}", 
            "tid", t->getId(), 
            "name", name.ansi(), 
            "cpu", t->cpuPercent(0), 
            "avg_cpu", t->cpuPercent(1), 
            "max_cpu", t->getMaxCpu(), 
            "priority", t->getPriority());//    {'abc': 123, 'def': 456}
         if(obj)
         {
   		   PyList_SetItem(list, i, obj);
         }
      }
	}
	return list;
}

static PyObject* rtm_getthreadids(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

	int max = p->numThreads();
   if(max==0)
      return PyLong_FromLong(0);

	PyObject*list = PyList_New(max);
   for(int i=0; i<max; i++)
	{
      THREAD* t = p->getThread(i);
      if(t)
      {
         PyList_SetItem(list, i, PyLong_FromLong( t->getId() ));
      }
	}
	return list;
}

static PyObject* rtm_getthreadname(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0;
	if(!PyArg_ParseTuple(args, "ii", &pid, &tid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   STRING name;
   name.set(t->getName() ? t->getName() : L"");

   return PyUnicode_FromString( name.ansi() );
}

static PyObject* rtm_setthreadname(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0;
	char *tname = 0;
	if(!PyArg_ParseTuple(args, "iis", &pid, &tid, &tname))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   STRING name;
   name.set(tname ? tname : "");
   t->setName(name.s());

	return PyLong_FromLong(1);
}

static PyObject* rtm_getthreadcpu(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0;
	if(!PyArg_ParseTuple(args, "ii", &pid, &tid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   return PyFloat_FromDouble(t->cpuPercent(0));
}

static PyObject* rtm_getthreadavgcpu(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0;
	if(!PyArg_ParseTuple(args, "ii", &pid, &tid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   return PyFloat_FromDouble(t->cpuPercent(1));
}

static PyObject* rtm_getthreadmaxcpu(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0;
	if(!PyArg_ParseTuple(args, "ii", &pid, &tid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   return PyFloat_FromDouble(t->getMaxCpu());
}

static PyObject* rtm_getthreadpriority(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0;
	if(!PyArg_ParseTuple(args, "ii", &pid, &tid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   int ret = t->getPriority();
	return PyLong_FromLong(ret);
}

//
static PyObject* rtm_addpstoreport(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   if (!p->getGenReport())
   {
	   p->setGenReport(true);
   }
	return PyLong_FromLong(1);
}

static PyObject* rtm_removepsfromreport(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   if (p->getGenReport())
   {
	   p->setGenReport(false);
   }

	return PyLong_FromLong(1);
}

static PyObject* rtm_addthtoreport(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0;
	if(!PyArg_ParseTuple(args, "ii", &pid, &tid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   if (!t->getGenReport())
   {
	   t->setGenReport(true);
   }

	return PyLong_FromLong(1);
}

static PyObject* rtm_removethfromreport(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0;
	if(!PyArg_ParseTuple(args, "ii", &pid, &tid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   if (t->getGenReport())
   {
	   t->setGenReport(false);
   }

	return PyLong_FromLong(1);
}
//
static PyObject* rtm_setthreadpriority(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0, priority=255;
	if(!PyArg_ParseTuple(args, "ii", &pid, &tid, &priority))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   int ret = device->sendSetThreadPriority(tid, priority);
	return PyLong_FromLong(ret);
}

static PyObject* rtm_crashwatchps(PyObject *self, PyObject *args)
{//pid, mapfile
	int pid = 0;
	char *mapfile = 0;
	if(!PyArg_ParseTuple(args, "i|s", &pid, &mapfile))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   STRING map;
   if(mapfile)
      map.set(mapfile);

   CDlgWatchCfg::StartCrashWatch(device, false, pid, NULL, map.s(), NULL, 
                                   NULL, NULL, false, true);

   return PyLong_FromLong(1);
}

static PyObject* rtm_crashwatchnewps(PyObject *self, PyObject *args)
{//pspath, mapfile
	char* path = 0, *mapfile = 0;
	if(!PyArg_ParseTuple(args, "s|s", &path, &mapfile))
		return PyLong_FromLong(0);

   if(!path)
      return PyLong_FromLong(0);

   STRING map;
   if(mapfile)
      map.set(mapfile);

   STRING remoteps;
   remoteps.set(path);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   CDlgWatchCfg::StartCrashWatch(device, true, 0, remoteps.s(), map.s(), NULL, 
                                   NULL, NULL, false, true);

   return PyLong_FromLong(1);
}

static PyObject* rtm_cpuwatch(PyObject *self, PyObject *args)
{
	int for_so_many_seconds = 0;
   float more_than_cpu_percent = 0;
	if(!PyArg_ParseTuple(args, "fi", &more_than_cpu_percent, &for_so_many_seconds))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   CDlgWatchCPUTotal::StartCPUWatch(device, more_than_cpu_percent, for_so_many_seconds, NULL, false);

	return PyLong_FromLong(1);
}

static PyObject* rtm_cpuwatchcancel(PyObject *self, PyObject *args)
{
   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   device->delTotalCPUWatch();

	return PyLong_FromLong(1);
}

static PyObject* rtm_cpuwatchps(PyObject *self, PyObject *args)
{
	int pid = 0, for_so_many_seconds = 0;
   float more_than_cpu_percent = 0;
	if(!PyArg_ParseTuple(args, "ifi", &pid, &more_than_cpu_percent, &for_so_many_seconds))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   CDlgWatchCPU::StartCPUWatch(p, device, more_than_cpu_percent, pid, 0, for_so_many_seconds, NULL, false, false, false);

	return PyLong_FromLong(1);
}

static PyObject* rtm_cpuwatchpscancel(PyObject *self, PyObject *args)
{
	int pid = 0;
	if(!PyArg_ParseTuple(args, "i", &pid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   p->delCPUWatch();

   return PyLong_FromLong(1);
}

static PyObject* rtm_cpuwatchth(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0, for_so_many_seconds = 0;
   float more_than_cpu_percent = 0;
	if(!PyArg_ParseTuple(args, "iifi", &pid, &tid, &more_than_cpu_percent, &for_so_many_seconds))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   CDlgWatchCPU::StartCPUWatch(t, device, more_than_cpu_percent, pid, tid, for_so_many_seconds, NULL, false, false, false);

	return PyLong_FromLong(1);
}

static PyObject* rtm_cpuwatchthcancel(PyObject *self, PyObject *args)
{
	int pid = 0, tid = 0;
	if(!PyArg_ParseTuple(args, "ii", &pid, &tid))
		return PyLong_FromLong(0);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   PROCESS* p = device->getProcessById(pid);
   if(!p)
      return PyLong_FromLong(0);

   THREAD* t = p->getThreadById(tid);
   if(!t)
      return PyLong_FromLong(0);

   t->delCPUWatch();

	return PyLong_FromLong(1);
}

static PyObject* rtm_memwatch(PyObject *self, PyObject *args)
{
	int less_than_bytes = 0, for_so_many_seconds = 0;
	if(!PyArg_ParseTuple(args, "iis", &less_than_bytes, &for_so_many_seconds))
		return PyLong_FromLong(0);

	return PyLong_FromLong(1);
}

static PyObject* rtm_memwatchcancel(PyObject *self, PyObject *args)
{
   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   DEVICE* device = mainWnd->GetDevice(0);
   if(!device)
      return PyLong_FromLong(0);

   device->delMemWatch();

   return PyLong_FromLong(1);
}

static PyObject* rtm_email(PyObject *self, PyObject *args)
{
	char* to = 0, *subject = 0, *msg = 0;
	if(!PyArg_ParseTuple(args, "sss", &to, &subject, &msg))
		return PyLong_FromLong(0);
   STRING to_, subject_, msg_;
   if(!to)
      return PyLong_FromLong(0);
   if(!subject)
      return PyLong_FromLong(0);
   if(!msg)
      return PyLong_FromLong(0);

   to_.set(to);
   subject_.set(subject);
   msg_.set(msg);

   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   mainWnd->SendEmail(to_, subject_, msg_);

   return PyLong_FromLong(1);
}

static PyObject* rtm_write(PyObject *self, PyObject *args)
{
	char *msg = 0;
	if(!PyArg_ParseTuple(args, "s", &msg))
		return PyLong_FromLong(0);

   if(msg && lstrlenA(msg))
   {
      CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
      mainWnd->m_python.Print(msg);
   }

   return PyLong_FromLong(1);
}

static PyObject* rtm_flush(PyObject *self, PyObject *args)
{
   return PyLong_FromLong(1);
}

//callback from RTM to Py
void 
py_got_pspath(DWORD pid, const char_t *name)
{
   PY_NOTIFY *notify = new PY_NOTIFY(PY_NOTIFICATION_GOT_PS_PATH, 0, 0, name, 0);
   if(notify)
      rtm_q.Enque(notify);
}

//callback from RTM to Py
void
py_got_snapshot()
{
   PY_NOTIFY *notify = new PY_NOTIFY(PY_NOTIFICATION_GOT_SNAPSHOT, 0, 0, 0, 0);
   if(notify)
      rtm_q.Enque(notify);
}

//callback from RTM to Py
void
py_crashwatch_ack(int pid, STRING &path, bool ok)
{
   const char_t* _path = 0;
   if(path.size() > 0)
      _path = path.s();
   PY_NOTIFY *notify = new PY_NOTIFY(ok ? PY_NOTIFICATION_CRASHWATCH_ACK : PY_NOTIFICATION_CRASHWATCH_NACK, pid, 0, _path, 0);
   if(notify)
      rtm_q.Enque(notify);
}

//callback from RTM to Py when a ps crashed on device
void 
py_got_exception(int pid, const STRING &path, const STRING &excmsg)
{
   PY_NOTIFY *notify = new PY_NOTIFY(PY_NOTIFICATION_EXCEPTION, pid, 0, path.s(), excmsg.s());
   if(notify)
      rtm_q.Enque(notify);
}

//callback from RTM to Py
void
py_memwatch_alert(STRING &msg)
{
   PY_NOTIFY *notify = new PY_NOTIFY(PY_NOTIFICATION_MEM_ALERT, 0, 0, 0, msg.s());
   if(notify)
      rtm_q.Enque(notify);
}

//callback from RTM to Py
void
py_totalcpuwatch_alert(STRING &msg)
{
   PY_NOTIFY *notify = new PY_NOTIFY(PY_NOTIFICATION_TOTAL_CPU_ALERT, 0, 0, 0, msg.s());
   if(notify)
      rtm_q.Enque(notify);
}

//callback from RTM to Py
void
py_pscpuwatch_alert(int pid, STRING &msg)
{
   PY_NOTIFY *notify = new PY_NOTIFY(PY_NOTIFICATION_PS_CPU_ALERT, pid, 0, 0, msg.s());
   if(notify)
      rtm_q.Enque(notify);
}

//callback from RTM to Py
void
py_thcpuwatch_alert(int pid, int tid, STRING &msg)
{
   PY_NOTIFY *notify = new PY_NOTIFY(PY_NOTIFICATION_TH_CPU_ALERT, pid, tid, 0, msg.s());
   if(notify)
      rtm_q.Enque(notify);
}

//callback from RTM to Py
void
py_notfiy_state(int state, STRING &py_msg)
{
   PY_NOTIFY *notify = 0;
	switch(state)
	{
	case CONNECTION_DATA_STATE__CONNECTED:
      notify = new PY_NOTIFY(PY_NOTIFICATION_CONNECTED, 0, 0, 0, 0);
		break;
	case CONNECTION_DATA_STATE__NONE:
	case CONNECTION_DATA_STATE__FAILED:
      notify = new PY_NOTIFY(PY_NOTIFICATION_DISCONNECTED, 0, 0, py_msg, 0);//py_msg is the path to index.html report for the last session
		break;
	}
   py_con_state = state;
   if(notify)
      rtm_q.Enque(notify);
}

static void
emptyq()
{
   while(rtm_q.GetCount())
   {
      PY_NOTIFY *notify = (PY_NOTIFY *)rtm_q.Deque();
      delete notify;
   }
   rtm_q.Destroy();
}

static void 
py_start(CDlgRemoteCPU *mainWnd)
{
     //Py_NoSiteFlag = 1;
     //PyEval_InitThreads();
     Py_Initialize();
     //PyEval_ReleaseLock();
     mainWnd->m_python.Print("Interpreting script...\n");
     py_initialized = true;
     emptyq();
}

void 
py_end(CDlgRemoteCPU *mainWnd)
{
   if(py_initialized)
   {
      PyImport_Cleanup();
      Py_Finalize();
      mainWnd->m_python.Print("Exit\n");
      py_initialized = false;
      emptyq();
   }
}

int
py_main(const char_t *pyfile)
{
   static bool py_init = false;
   CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
   mainWnd->m_python.Clear();
   mainWnd->SetProcessingScript(true);

   py_end(mainWnd);

   if(pyfile)
   {
      STRING in(pyfile);
      in.trim(L" \t");
	  char_t* dot_ = in.strrchr('.');
	  char_t* slash_ = in.strrchr('\\');
	  int dot = dot_ ? dot_ - in.s() : 0;
	  int slash = slash_ ? slash_ - in.s() : 0;
      STRING path;
      in.substr(path, 0, slash+1);
      STRING file;
      in.substr(file, slash+1, dot-slash-1);

      char_t dir[MAX_DIR_PATH];
      if(GetCurrentDirectory(sizeof(dir)/sizeof(wchar_t), dir))
      {
         if(!path.size())
            path.sprintf(L"%s\\", dir);
      }
      if(!file.size() || !path.size())
      {
         STRING msg;
         msg.sprintf(L"Could not locate Python script! \"%s\"\n", pyfile);
         mainWnd->m_python.Print(msg.ansi());
      }
      else
      {
        py_start(mainWnd);

        PyImport_AppendInittab("rtm", &PyInit_rtm);

         PyObject *main = PyImport_AddModule("__main__");

         PyRun_SimpleString("import sys");

         PyRun_SimpleString("import rtm");
         PyRun_SimpleString("sys.stderr = rtm");
         PyRun_SimpleString("sys.stdout = rtm");

         STRING path2;
         path.replace(L'\\', L'/', -1);
         path2.sprintf(L"sys.path.append(\"%s\")\n", path.s());
         PyRun_SimpleString(path2.ansi());
         PyErr_Print();
         ////
         PyObject *obj = PyUnicode_FromString(file.ansi());
         PyModule = PyImport_Import(obj);
         PyErr_Print();
         Py_DECREF(obj);
#if 1
         if(!PyModule)
         {
            STRING err;
            err.sprintf(L"Failed to interpret \"%s\".\n", in.s());
            mainWnd->m_python.Print(err.ansi());
         }
         else 
         {
            rtm_call(PyModule, "main", 0, 0);

            Py_DECREF(PyModule);
         }
#endif
         py_end(mainWnd);
      }
   }
   mainWnd->SetProcessingScript(false);
	return 0;
}
