// DlgDbgPsInfo.cpp : implementation file
//

#include "stdafx.h"


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
#include "dlgDbgPsInfo.h"
#include "DlgRemoteCPU.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPsInfo dialog


CDlgDbgPsInfo::CDlgDbgPsInfo(DEVICE * device, CRASH_INFO *cinfo, STRING &mapfile, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDbgPsInfo::IDD, pParent)
	, m_device(device)
   , m_cinfo(0)
   , m_mapfile(mapfile)
{
   copy_cinfo(cinfo);
	//{{AFX_DATA_INIT(CDlgDbgPsInfo)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CDlgDbgPsInfo::~CDlgDbgPsInfo()
{
   if(m_cinfo)
   {
      free(m_cinfo);
   }
}

bool CDlgDbgPsInfo::copy_cinfo(CRASH_INFO *cinfo)
{
   bool ret = false;
   if(cinfo)
   {
      int num_frames = cinfo->msg.numelm;
      int sz = sizeof(CRASH_INFO) + sizeof(CallSnapshotEx) * num_frames;
      if(m_cinfo)
         free(m_cinfo);
      m_cinfo = (CRASH_INFO*)malloc(sz);
      if(m_cinfo)
      {
         memcpy(m_cinfo, cinfo, sizeof(CRASH_INFO));
         for(int i=0; i<num_frames; i++)
         {
            memcpy(&m_cinfo->frame[i], &cinfo->frame[i], sizeof(CallSnapshotEx));
         }
         ret = true;
      }
   }
   return ret;
}

void CDlgDbgPsInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDbgPsInfo)
	DDX_Control(pDX, IDC_EDIT_MESG, m_edit_ps_info);
   DDX_Control(pDX, IDC_EDIT_MAP_FILE_PATH, m_edit_map_file_path);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDbgPsInfo, CDialog)
	//{{AFX_MSG_MAP(CDlgDbgPsInfo)
	ON_WM_DESTROY()
   ON_BN_CLICKED(ID_PARSE_MAP_FILE, OnParseMapFile)
   ON_BN_CLICKED(ID_GET_MAP_FILE, OnButtonMapFilePath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPsInfo message handlers

void CDlgDbgPsInfo::OnDestroy()
{
   CDialog::OnDestroy();
   delete this;
}

void CDlgDbgPsInfo::OnOK() 
{
   CDialog::OnOK();
   DestroyWindow();
}

void CDlgDbgPsInfo::OnCancel() 
{
   CDialog::OnCancel();
   DestroyWindow();

   AfxGetMainWnd()->SetTimer(CANCEL_PS_WATCHES, 100, 0);
}

BOOL CDlgDbgPsInfo::OnInitDialog() 
{
   BOOL ret = CDialog::OnInitDialog();

   if(ret)
   {
      if(m_mapfile.size())
         m_edit_map_file_path.SetWindowText(m_mapfile.s());
      else
         m_edit_map_file_path.SetWindowText(L"");

      if(m_cinfo->msg.numelm > 0)
         GetDlgItem(IDC_STATIC_CALLSTACK_DISCLAIMER)->ShowWindow(TRUE);
   }
   return ret;
}

void CDlgDbgPsInfo::OnButtonMapFilePath()
{
   CString path;
   if(getMapFilePath(path))
   {
      CWnd * editpath = GetDlgItem(IDC_EDIT_MAP_FILE_PATH);
      editpath->SetWindowText(path);
   }
}

void CDlgDbgPsInfo::OnParseMapFile()
{
   STRING msg;
   UpdateData(TRUE);
   CWnd * mappath = GetDlgItem(IDC_EDIT_MAP_FILE_PATH);
   if(mappath)
   {
      CString path;
      mappath->GetWindowText(path);
      m_mapfile.set(path);
      if(getExceptionMsg(msg))
         m_edit_ps_info.SetWindowText(msg.size() ? msg.s() : L"");
   }
}

int CDlgDbgPsInfo::getExceptionMsg(STRING &msg)
{
   int ret = 0;
   STRING tmp, excode;
   EXCEPTION_DEBUG_INFO* exception = 0;
   ADDR_INFO *addr_info = 0, *addr = 0;
   int num_addrs = 0;
   THREAD *t = 0;
   PROCESS *p = m_device ? m_device->getProcessById(m_cinfo->debug_event.dwProcessId) : 0;

   if(p)
   {
	   WATCH_ITEM *watch = p->getCrashWatch();
	   m_device->removeWatch(watch);
	   p->setCrashWatch(0);
   }

   if(!m_cinfo)
      goto Exit;

   if(m_cinfo->msg.numelm < 0)
      m_cinfo->msg.numelm = 0;

   exception = &m_cinfo->debug_event.u.Exception;

   num_addrs = m_cinfo->msg.numelm + 1;
   addr_info = new ADDR_INFO[num_addrs];
   if(addr_info)
   {
      int i=0;
      addr_info[i].set((DWORD)exception->ExceptionRecord.ExceptionAddress, tmp, 0, tmp);
      i++;

      int j=0;
      while(j < m_cinfo->msg.numelm)
      {
         addr_info[i].set(m_cinfo->frame[j].dwReturnAddr, tmp, 0, tmp);
         i++;
         j++;
      }
   }

   if(m_mapfile.size())
   {
      parseMapFile(m_mapfile, addr_info, num_addrs);
   }

   getExceptionCodeAsString(exception->ExceptionRecord.ExceptionCode, excode);

   t = p ? p->getThreadById(m_cinfo->debug_event.dwThreadId) : 0;
   addr = getAddrInfo1((int)exception->ExceptionRecord.ExceptionAddress, addr_info, num_addrs);
   msg.sprintf(L"%s[pid:0x%x] %s[tid:0x%x]\r\nException Address:[0x%x] %s + 0x%x\t%s\r\nException Code:0x%x %s\r\n\r\n",
      p && p->getName() ? p->getName() : L"",
      m_cinfo->debug_event.dwProcessId,
      t && t->getName() ? t->getName() : L"",
      m_cinfo->debug_event.dwThreadId,
      exception->ExceptionRecord.ExceptionAddress,
      addr && addr->m_funcname.size() ? addr->m_funcname.s() : L"",
      addr ? addr->m_offset : 0,
      addr && addr->m_objfile.size() ? addr->m_objfile.s() : L"",
      exception->ExceptionRecord.ExceptionCode,
      excode.s()
      );
   if(m_cinfo->msg.numelm > 0)
   {
	   msg.append(L"Frame Pointer\tReturn Address\tProcess & Function Name\tLibrary & Object File\tParam 0\tParam 1\tParam 2\tParam 3\r\n\r\n");
	   if(m_cinfo->msg.numelm <= MAX_FRAMES)
	   {
		   int i;
		   for(i=0; i<m_cinfo->msg.numelm; i++)
		   {
			   PROCESS *p2 = m_device ? m_device->getProcessById(m_cinfo->frame[i].dwCurProc) : 0;
			   addr = getAddrInfo1(m_cinfo->frame[i].dwReturnAddr, addr_info, num_addrs);
			   tmp.sprintf(L"0x%x\t0x%x\t%s[0x%x] %s + 0x%x\t%s\t0x%x\t0x%x\t0x%x\t0x%x\t\r\n", 
				   m_cinfo->frame[i].dwFramePtr, 
				   m_cinfo->frame[i].dwReturnAddr,
				   p2 && p2->getName() ? p2->getName() : L"",
				   m_cinfo->frame[i].dwCurProc,
				   addr && addr->m_funcname.size() ? addr->m_funcname.s() : L"",
				   addr ? addr->m_offset : 0,
				   addr && addr->m_objfile.size() ? addr->m_objfile.s() : L"",
				   m_cinfo->frame[i].dwParams[0],
				   m_cinfo->frame[i].dwParams[1],
				   m_cinfo->frame[i].dwParams[2],
				   m_cinfo->frame[i].dwParams[3]);
			   
			   msg.append(tmp);
			   tmp.clear();
		   }
	   }
   }

   if(addr_info)
      delete [] addr_info;
Exit:
   return exception->ExceptionRecord.ExceptionCode;
}

int CDlgDbgPsInfo::parseMapFile(STRING &mapfile, ADDR_INFO *out_addr, int num_addrs)
{
   int ret =0 ;
   STRING line;
   const int max_tokens = 100;
   int num_tokens = max_tokens;
   FILE *fp;
   int ln;
   STRING *token;
   bool in_proc_list = false;
   long faddr;
   STRING psname, libobj, funcaddr, funcname;
   char buffer[1024];
   char_t *endptr = 0 ;

   token = new STRING [max_tokens];
   if(!token)
      goto Exit;

   fp = fopen(mapfile.ansi(), "rt");
   if(!fp)
      goto Exit;
   for(ln = 0; fgets(buffer, sizeof(buffer), fp); ln++)
   {
      line.set(buffer);

      if(ln==0)
      {
         psname.set(line);
      }

      num_tokens = max_tokens;
      for(int n=0; n<max_tokens; n++)
         token[n].clear();

      if(line.explode(TXT(" "), token, num_tokens))
      {
         if(!in_proc_list && 
            token[max_tokens-2].size() && 
            wcsstr(token[max_tokens-2].s(), TXT("Rva+Base")))
         {
            in_proc_list = true;
         }
         else if(in_proc_list && wcsstr(line.s(), TXT("entry point at")))
         {
            in_proc_list = false;
         }
         else if(in_proc_list && line.size()>0)
         {
            //             Address         Publics by Value              Rva+Base       Lib:Object
            // 
            //                0000:00000000       ___safe_se_handler_table   00000000     <absolute>
            //                0000:00000000       ___safe_se_handler_count   00000000     <absolute>
            //                0001:00000000       ?crash@@YAXXZ              00011000 f   crashps.obj
            //                0001:00000020       ?foo2@@YAXH@Z              00011020 f   crashps.obj
            //                0001:0000003c       ?foo1@@YAXHPAD@Z           0001103c f   crashps.obj
            //                0001:0000005c       wmain                      0001105c f   crashps.obj
            //                0001:00000518       mainWCRTStartup            00011518 f   corelibc:mainwcrt.obj
            //                0001:0000053c       free                       0001153c f   corelibc:COREDLL.dll

            int k=1;

            //Lib:Object (file name)
            if(k <= max_tokens)
            {
               libobj.set(token[max_tokens-k]);
               k++;
            }

            //Rva+Base (function address), or f or i
            faddr = 0;
            while(k <= max_tokens)
            {
               funcaddr.set(token[max_tokens-k]);
               k++;
               if(funcaddr.size() > 4)
                  faddr = wcstol(funcaddr.s(), &endptr, 16);
               if(faddr>0)
                  break;
            }

            //Publics by Value (function name)
            if(k <= max_tokens)
            {
               funcname.set(token[max_tokens-k]);
               k++;
            }

            funcaddr.trim(TXT(" "));
            funcaddr.rtrim(TXT("\n"));
            
            funcname.trim(TXT(" "));
            funcname.rtrim(TXT("\n"));
            
            libobj.trim(TXT(" "));
            libobj.rtrim(TXT("\n"));

            if(funcaddr.size())
            {
               int i;
               char_t *endptr = 0 ;
               faddr = wcstol(funcaddr.s(), &endptr, 16);

               for(i=0; i<num_addrs && faddr>0; i++)
               {
                  if(out_addr[i].m_address > faddr)
                  {
                     out_addr[i].m_funcname.set(funcname.s());
                     out_addr[i].m_objfile.set(libobj.s());
                     out_addr[i].m_offset = out_addr[i].m_address - faddr;
                  }
               }
            }
         }

      }
   }
   ret = 1;
Exit:
   if(token)
      delete [] token;
   token = 0;
   if(fp)
      fclose(fp);
   fp= 0;
   return ret;
}

ADDR_INFO *CDlgDbgPsInfo::getAddrInfo1(int addr, ADDR_INFO *list, int count)
{
   int j;
   for(j=0; j<count; j++)
   {
      if(list[j].m_address == addr)
         return &list[j];
   }
   return 0;
}

bool CDlgDbgPsInfo::getMapFilePath(CString &path)
{
   bool ret = false;
   static char_t BASED_CODE g_szFilter[] = L"Visual Studio Map File (*.map)|*.map|All Files (*.*)|*.*||";
   CFileDialog m_fileDialog(TRUE, 0, 0, OFN_ENABLESIZING|OFN_PATHMUSTEXIST|OFN_NONETWORKBUTTON, //|OFN_ALLOWMULTISELECT
      g_szFilter);

   char_t dir[MAX_DIR_PATH];
   if(GetCurrentDirectory(sizeof(dir)/sizeof(char_t), dir))
   {
      m_fileDialog.m_ofn.lpstrInitialDir=dir;
   }

   if(IDOK==m_fileDialog.DoModal())
   {
      path=m_fileDialog.GetPathName();
      ret = true;
   }
   else
   {
      path = L"";
   } 
   return ret;
}

void CDlgDbgPsInfo::getExceptionCodeAsString(ULONG32 exceptionCode, STRING &ret)
{
   switch(exceptionCode)
   {
   case EXCEPTION_ACCESS_VIOLATION:
      ret.set(L"EXCEPTION_ACCESS_VIOLATION");
      break;
   case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      ret.set(L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
      break;
   case EXCEPTION_BREAKPOINT:
      ret.set(L"EXCEPTION_BREAKPOINT");
      break;
   case EXCEPTION_DATATYPE_MISALIGNMENT:
      ret.set(L"EXCEPTION_DATATYPE_MISALIGNMENT");
      break;
   case EXCEPTION_FLT_DENORMAL_OPERAND:
      ret.set(L"EXCEPTION_FLT_DENORMAL_OPERAND");
      break;
   case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      ret.set(L"EXCEPTION_FLT_DIVIDE_BY_ZERO");
      break;
   case EXCEPTION_FLT_INEXACT_RESULT:
      ret.set(L"EXCEPTION_FLT_INEXACT_RESULT");
      break;
   case EXCEPTION_FLT_INVALID_OPERATION:
      ret.set(L"EXCEPTION_FLT_INVALID_OPERATION");
      break;
   case EXCEPTION_FLT_OVERFLOW:
      ret.set(L"EXCEPTION_FLT_OVERFLOW");
      break;
   case EXCEPTION_FLT_STACK_CHECK:
      ret.set(L"EXCEPTION_FLT_STACK_CHECK");
      break;
   case EXCEPTION_FLT_UNDERFLOW:
      ret.set(L"EXCEPTION_FLT_UNDERFLOW");
      break;
   case EXCEPTION_ILLEGAL_INSTRUCTION:
      ret.set(L"EXCEPTION_ILLEGAL_INSTRUCTION");
      break;
   case EXCEPTION_IN_PAGE_ERROR:
      ret.set(L"EXCEPTION_IN_PAGE_ERROR");
      break;
   case EXCEPTION_INT_DIVIDE_BY_ZERO:
      ret.set(L"EXCEPTION_INT_DIVIDE_BY_ZERO");
      break;
   case EXCEPTION_INT_OVERFLOW:
      ret.set(L"EXCEPTION_INT_OVERFLOW");
      break;
   case EXCEPTION_INVALID_DISPOSITION:
      ret.set(L"EXCEPTION_INVALID_DISPOSITION");
      break;
   case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      ret.set(L"EXCEPTION_NONCONTINUABLE_EXCEPTION");
      break;
   case EXCEPTION_PRIV_INSTRUCTION:
      ret.set(L"EXCEPTION_PRIV_INSTRUCTION");
      break;
   case EXCEPTION_SINGLE_STEP:
      ret.set(L"EXCEPTION_SINGLE_STEP");
      break;
   case EXCEPTION_STACK_OVERFLOW:
      ret.set(L"EXCEPTION_STACK_OVERFLOW");
      break;
   default:
      ret.set(L" ");
      break;
   }
}