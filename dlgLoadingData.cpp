// CDlgLoadingData.cpp : implementation file
//

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../common/types.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "gdgraph.h"
#include "history.h"
#include "watchlist.h"
#include "device.h"
#include "RemoteCPU.h"
#include "DlgLoadingData.h"
#include "DlgRemoteCPU.h"
#include "watchlist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadingData dialog


CDlgLoadingData::CDlgLoadingData(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgLoadingData::IDD, pParent)
   , m_closed(false)
{
	//{{AFX_DATA_INIT(CDlgLoadingData)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgLoadingData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLoadingData)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgLoadingData, CDialog)
	//{{AFX_MSG_MAP(CDlgLoadingData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadingData message handlers

BOOL CDlgLoadingData::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd *progressWnd = GetDlgItem(IDC_PROGRESS_LOAD_DATA);
	progressWnd->SendMessage(PBM_SETRANGE, 0, MAKELPARAM(0, 100)); 
	progressWnd->SendMessage(PBM_SETSTEP, 1, 0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CDlgLoadingData::loadBegin(DEVICE &device, STRING &file, HISTORY &history, FILE **fpp, STRING &err, int &total_read)
{
   int ret=0;
   int sz = 0, i, num;
   COMM_MESG *msg = 0;
   COMM_INIT_SETTINGS *init_settings = 0;
   MY_SYSTEM_INFO *si = 0;
   THREAD_NAME *tn = 0;
   FILE* fp = 0;
   *fpp = 0;

   history.Start(file.s());

   fp = fopen(file.ansi(), "rb");
   if(!fp)
      goto Exit;

   /////////////////////////////////////////// COMM_CMD_INIT_SETTINGS
   ret=fread(&sz, 1, sizeof(sz), fp);
   if(!ret)
      goto Exit;
   total_read+=ret;
   if(sz<=0)
      goto Exit;
   msg = (COMM_MESG *)malloc(sz);
   if(!msg)
      goto Exit;
   ret=fread(msg, 1, sz, fp);
   if(!ret)
      goto Exit;
   total_read+=ret;
   init_settings = (COMM_INIT_SETTINGS *)(&msg[1]);
   free(msg);
   msg = 0;
   /////////////////////////////////////////// COMM_CMD_SYSINFO
   ret=fread(&sz, 1, sizeof(sz), fp);
   if(!ret)
      goto Exit;
   total_read+=ret;
   if(sz<=0)
      goto Exit;
   msg = (COMM_MESG *)malloc(sz);
   if(!msg)
      goto Exit;
   ret=fread(msg, 1, sz, fp);
   if(!ret)
      goto Exit;
   total_read+=ret;
   si = (MY_SYSTEM_INFO *)(&msg[1]);

   if(!device.checkVersion(msg->id, RTMON_VERSION, err))
   {
      goto Exit;
   }
   memcpy(device.getSysInfo(), si, sizeof(MY_SYSTEM_INFO));
   free(msg);
   msg = 0;

   /////////////////////////////////////////// THREAD_NAME
   ret = fread(&sz, 1, sizeof(sz), fp);
   if(!ret)
      goto Exit;
   total_read+=ret;
   if(sz<=0)
      goto Exit;
   msg = (COMM_MESG *)malloc(sz);
   if(!msg)
      goto Exit;
   ret = fread(msg, 1, sz, fp);
   if(!ret)
      goto Exit;
   total_read+=ret;
   tn = (THREAD_NAME *)(&msg[1]);
   num = msg->numelm;
   for(i=0; i<msg->numelm; i++)
   {
      device.setThreadName(tn[i].tid, tn[i].name);
   }
   free(msg);
   msg = 0;

   ret = 1;
   *fpp = fp;
Exit:
   if(msg)
      free(msg);
   return ret;
}

int CDlgLoadingData::loadSome(DEVICE &device, HISTORY &history, FILE *fp, STRING &err, int &total_read, CDlgRemoteCPU* parent)
{
   int ret=0;
   int sz = 0, num = 0, i;
   COMM_MESG *msg = 0;
   COMM_SNAPINFO *sn = 0;
   EXECOBJ_DATA *pd = 0;
   EXECOBJ_DATA *td = 0;

   if(fp && !feof(fp))
   {
      /////////////////////////////////////////// COMM_SNAPINFO
      ret = fread(&sz, 1, sizeof(sz), fp);
      if(!ret)
         goto Exit;
      total_read+=ret;
      if(sz<=0)
         goto Exit;
      msg = (COMM_MESG *)malloc(sz);
      if(!msg)
         goto Exit;
      ret = fread(msg, 1, sz, fp);
      if(!ret)
         goto Exit;
      total_read+=ret;
      sn = (COMM_SNAPINFO *)(&msg[1]);
      num = msg->numelm;
      for(i=0; i<num; i++)
      {
         device.addSnapInfo(&sn[i]);
      }
      free(msg);
      msg = 0;
	  device.removeDeadProcesses();

      /////////////////////////////////////////// COMM_CMD_PDATA
      ret = fread(&sz, 1, sizeof(sz), fp);
      if(!ret)
         goto Exit;
      total_read+=ret;
      if(sz<=0)
         goto Exit;
      msg = (COMM_MESG *)malloc(sz);
      if(!msg)
         goto Exit;
      ret = fread(msg, 1, sz, fp);
      if(!ret)
         goto Exit;
      total_read+=ret;
      pd = (EXECOBJ_DATA *)(&msg[1]);
      num = msg->numelm;
      for(i=0; i<num; i++)
      {
         PROCESS* p = device.addProcess(&pd[i], &device);
		 p->setGenReport(true);
      }
      free(msg);
      msg = 0;

      /////////////////////////////////////////// COMM_CMD_TDATA
      ret = fread(&sz, 1, sizeof(sz), fp);
      if(!ret)
         goto Exit;
      total_read+=ret;
      if(sz<=0)
         goto Exit;
      msg = (COMM_MESG *)malloc(sz);
      if(!msg)
         goto Exit;
      ret = fread(msg, 1, sz, fp);
      if(!ret)
         goto Exit;
      total_read+=ret;
      td = (EXECOBJ_DATA *)(&msg[1]);
      num = msg->numelm;
      for(i=0; i<num; i++)
      {
         THREAD *t = device.addThread(&td[i]);
		 t->setGenReport(true);
      }
      free(msg);
      msg = 0;

      history.Write(&device, parent);
   }
   ret = 1;
Exit:
   if(msg)
      free(msg);
   return ret;
}

int CDlgLoadingData::loadEnd(DEVICE &device, HISTORY &history, FILE *fp, STRING &err, CDlgRemoteCPU* parent, STRING &outdir)
{
   int ret=0;
   ret = history.Stop(&device, outdir, parent);
   if(fp)
      fclose(fp);
   return ret;
}

int CDlgLoadingData::start(CDlgRemoteCPU* parent, DEVICE &device, STRING &file, HISTORY &history, STRING &outdir)
{
/*   hDialog=CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_VERIFY_USER), hwnd, (DLGPROC)Dialog_VerifyUser);
   if(!hDialog)
   {
      int ret=GetLastError();
      return false;
   }
*/
   int ret = 0;
   if(Create(IDD_LOADING_DATA, (CWnd*)parent))
   {
      CWnd *progressWnd = GetDlgItem(IDC_PROGRESS_LOAD_DATA);
      CWnd *staticWnd = GetDlgItem(IDC_STATIC_LOADING_RTM);
      int cond = 0;
      STRING err, title, outfile;
      FILE *fp = 0;
      int total_read = 0;
      int file_sz = 0;

      ShowWindow(SW_SHOW);

      title.set(L"   ");
      title.sprintf(L"Loading \"%s\". Please wait...", file.s());
      staticWnd->SetWindowText(title.s());

      struct stat st;
      stat(file.ansi(), &st);
      file_sz = st.st_size;

      cond = loadBegin(device, file, history, &fp, err, total_read) && fp ? 
         1:0;

	  if(!cond && err.size())
	  {
		  CDlgRemoteCPU *mainWnd = (CDlgRemoteCPU *)AfxGetMainWnd();
		  mainWnd->m_dlgerr.SetMessage(STRING(L"RTM Loading Error"), err, STRING(TXT("")));
	  }

      while(cond)
      {
         cond = loadSome(device, history, fp, err, total_read, parent);

         progressWnd->SendMessage(PBM_SETPOS, total_read*100/file_sz);

         if(1)
         {
            int ret1 = parent->ProcessWinMessages(this);
            switch(ret1)
            {
            case -1:
               break;
            case 0:
               goto Exit;
            //case 1://we processed a message, continue doing that
            }
            if(m_closed)
               goto Exit;
         }

         if(total_read >= file_sz)
         {
            DestroyWindow();

            cond=false;
         }
      }
      ret = loadEnd(device, history, fp, err, parent, outdir);

      if(ret)
      {
         file.set(outfile.s());
      }
   }
Exit:
   return ret;
}

void CDlgLoadingData::OnCancel() 
{
   m_closed=true;
   DestroyWindow();
}