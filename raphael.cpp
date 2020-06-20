#include "stdafx.h"
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
//#define USE_HISTORY_DATA
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "gdgraph.h"
#include "history.h"
#include "watchlist.h"
#include "device.h"
#include "RemoteCPU.h"
#include "dlgLoadingData.h"
#include "dlgRemotecpu.h"
#include "resource.h"
#include "Raphael.h"

#ifdef USE_HISTORY_DATA
enum
{
   HTML_1,
   HTML_2,
   HTML_3,
   STYLE_CSS,
   PLOT_JS,
   RAPHAEL_JS,
   HEADERBG_PNG,
   PBG_PNG,
   LOGO_PNG,
   NUM_RES,
};
#endif//USE_HISTORY_DATA

char_t RES_FILE_NAME[][64] = {
	L"", L"", L"", L"style.css", L"plot.js", L"raphael.js", L"headerbg.png", L"pbg.png", L"remotetaskmontorlogo.png"
};

Raphael::~Raphael()
{
}

Raphael::Raphael()
{
}

class CDialog1 : public CDialog
{
   bool m_closed;
public:
   CDialog1() : CDialog(), m_closed(false) {}
   void OnCancel();
   bool IsClosed() { return m_closed; }
};

void CDialog1::OnCancel() 
{
   m_closed=true;
   DestroyWindow();
}

bool Raphael::draw(
                   DEVICE * device, 
                   STRING &outdir, 
                   CDlgRemoteCPU* parent,
                   LIST *history_tags,
                   DATA_QUE &labels,
                   DATA_QUE &cpu,
                   DATA_QUE &mem
)
{
   bool ret = false;
#ifdef USE_HISTORY_DATA
   FILE* fp;
   STRING html, tmp;
   HRSRC hrsc[NUM_RES];
   HGLOBAL hglob[NUM_RES];
   MY_SYSTEM_INFO* si=0;
   MEMORYSTATUS* memst=0;
   STRING sysinfo, memtotal, memavail;
   const char* html_dt[NUM_RES];
   int html_sz[NUM_RES];
   int i, j, k;
   int num_ps = history_tags[0].GetCount();
   int num_th = history_tags[1].GetCount();
   int num_canvases = num_ps + num_th;
   STRING html_fname;
   STRING obj;
   HISTORY_DATA *h;
   ITERATOR it;
   STRING pname, tname, slabels, title, data, selectbox;
   CDialog1 dlg;
   CWnd *progressWnd, *staticWnd;
   int cond = 0;
   DWORD err=0;
   
   if(!dlg.Create(IDD_LOADING_DATA, (CWnd*)parent))
      goto Exit;

   progressWnd = dlg.GetDlgItem(IDC_PROGRESS_LOAD_DATA);
   staticWnd = dlg.GetDlgItem(IDC_STATIC_LOADING_RTM);

   dlg.ShowWindow(SW_SHOW);

   title.set(L"   ");
   title.sprintf(L"Generating report. Please wait...");
   staticWnd->SetWindowText(title.s());

   si = device->getSysInfo();
   memst = device->getMemStatus();

   for(i=0; i<NUM_RES; i++)
   {
	   switch(i)
	   {
	   case HTML_1:
		   hrsc[i] = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_HTML1), MAKEINTRESOURCE(RT_HTML));
		   break;
	   case HTML_2:
		   hrsc[i] = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_HTML2), MAKEINTRESOURCE(RT_HTML));
		   break;
	   case HTML_3:
		   hrsc[i] = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_HTML3), MAKEINTRESOURCE(RT_HTML));
		   break;
	   case STYLE_CSS:
		   hrsc[i] = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_CSS), MAKEINTRESOURCE(RT_HTML));
		   break;
	   case PLOT_JS:
		   hrsc[i] = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_PLOT_JS), MAKEINTRESOURCE(RT_HTML));
		   break;
	   case RAPHAEL_JS:
		   hrsc[i] = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_RAPHAEL_JS), MAKEINTRESOURCE(RT_HTML));
		   break;
	   case HEADERBG_PNG:
			hrsc[i] = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_HEADERBG_PNG), L"PNG");
			break;
	   case LOGO_PNG:
			hrsc[i] = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_LOGO_PNG), L"PNG");
			break;
	   case PBG_PNG:
			hrsc[i] = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_PBG_PNG), L"PNG");
			break;
	   }
	   if(!hrsc[i])
		   err=GetLastError();
	   else
	   {
		   hglob[i] = LoadResource(::AfxGetInstanceHandle(), hrsc[i]);
		   if(hglob[i])
		   {
				html_dt[i] = (char*)LockResource(hglob[i]);
				html_sz[i] = SizeofResource(::AfxGetInstanceHandle(), hrsc[i]);

			   if(RES_FILE_NAME[i][0])
			   {
				   tmp.set(outdir.s());
				   tmp.append(RES_FILE_NAME[i]);
				   fp = fopen(tmp.ansi(), "wb");
				   if(fp)
				   {
					   fwrite(html_dt[i], html_sz[i], 1, fp);
					   fclose(fp);
				   }
			   }
		   }
	   }
   }

   for(k=0; k<2; k++)
   {
	   h = (HISTORY_DATA*)history_tags[k].GetFirst(it);
	   for(i=0; h; i++)
	   {
		   if(k==0)
		   {
               pname.set(h->m_pname);
               obj.sprintf(L"pid%u", h->pid);
               title.sprintf(L"%s [pid:%i]", 
				   pname.s() ? pname.s() : L"", h->pid);

			   if(i==0)
			   {
				   tmp.sprintf(L"&nbsp;&nbsp;<select id=\"selbox-pid%i\" class=\"selectmult\" onchange=\"change_category_selbox(this.options[this.selectedIndex].value)\"><option value=\"0\"   >Processes</option>",
					   h->pid);
				   selectbox.append(tmp.s());
			   }
			   tmp.sprintf(L"\t\t\t<option value=\"pid%i\"   >%s</option>\n",
				   h->pid, title.s());
			   selectbox.append(tmp.s());
		   }
		   else
		   {
               pname.set(h->m_pname);
               tname.set(h->m_tname);
               obj.sprintf(L"pid%u-tid%u", h->pid, h->tid);
               title.sprintf(L"%s [pid:%i] %s [tid:%i] [priority:%i]", 
				   pname.s() ? pname.s() : L"", 
				   h->pid, 
				   tname.s() ? tname.s() : L"", 
				   h->tid, 
				   h->priority);

			   if(i==0)
			   {
				   tmp.sprintf(L"&nbsp;&nbsp;<select id=\"selbox-pid%i-tid%i\" class=\"selectmult\" onchange=\"change_category_selbox(this.options[this.selectedIndex].value);\"><option value=\"0\"   >Threads</option>",
					   h->pid, h->tid);
				   selectbox.append(tmp.s());
			   }
			   tmp.sprintf(L"\t\t\t<option value=\"pid%i-tid%i\"   >%s</option>\n",
				   h->pid, h->tid, title.s());
			   selectbox.append(tmp.s());
		   }
		   h = (HISTORY_DATA*)history_tags[k].GetNext(it);
		   if(!h && i>0)
		   {
			   selectbox.append(L"</select>");
		   }
	   }
   }

   for(j=0; j<num_canvases+1; j++)
   {
      progressWnd->SendMessage(PBM_SETPOS, j*100/num_canvases+1);

      if(1)
      {
         int ret1 = parent->ProcessWinMessages(&dlg);
         switch(ret1)
         {
         case -1:
            break;
         case 0:
            goto Exit;
            //case 1://we processed a message, continue doing that
         }
      }
      if(dlg.IsClosed())
      {
         goto Exit;
      }

      html.set(html_dt[HTML_1], html_sz[HTML_1]);
      html.append(selectbox.s());
      tmp.set(html_dt[HTML_2], html_sz[HTML_2]);
      html.append(tmp.s(), tmp.size());

      if(j==0)//first
      {
         slabels.set(labels.labels().s());

         html_fname.set(outdir.s());
         html_fname.append(L"index.html");

         html.append(L"\n\
                      <script type=\"text/javascript\" charset=\"utf-8\">\n\
                         var labels = new Array(");
         html.append(slabels.s());
         html.append(L");\n\
                         var mem = new Array(");
         html.append(mem.mem().s());
         html.append(L");\n\
                         var cpu = new Array(");
         html.append(cpu.cpu().s());
         html.append(L");\n\
                         window.onload = function ()\n\
                         {\n\
                           drawGraph(\"canvas_cpu\", cpu, labels);\n\
                           drawGraph(\"canvas_mem\", mem, labels);\n\
                         };\n\
                      </script>\n\
                      <div class=\"summary\">\n\
                         Number of Processors: ");
         tmp.sprintf(L"%i", si->numberOfProcessors());
         html.append(tmp.s());
         html.append(L", \n\
                         Page size: ");
         tmp.sprintf(L"%i", si->dwPageSize);
         html.append(tmp.s());
         html.append(L", \n\
                         Memory total: ");
         html.append(memtotal.s());
         html.append(L"<br/>\n\
                      </div>\n\
                      <br/>\n\
                      <b>Overall CPU %</b><br/>\n\
                      <div class=\"graph\">\n\
                         <div id=\"canvas_cpu\"></div>\n\
                      </div>\n\
                      <b>Overall Memory %</b><br/>\n\
                      <div class=\"graph\">\n\
                         <div id=\"canvas_mem\"></div>\n\
                      </div>\n\
					 <br/><br/>\n\
                      ");
      }//if(j==0)//first
      else
      {
         html_fname.set(outdir.s());
         h=0;
         
         if(j>0 && j<num_ps+1)
         {
            if(j==1)
               h = (HISTORY_DATA*)history_tags[0].GetFirst(it);
            else
               h = (HISTORY_DATA*)history_tags[0].GetNext(it);
            if(h)
            {
               pname.set(h->m_pname);
               obj.sprintf(L"pid%u", h->pid);
               title.sprintf(L"%s [pid:%i]", 
                  pname.s() ? pname.s() : L"", h->pid);
            }
         }
         else if(j>num_ps && j<num_canvases+1)
         {
            if(j==num_ps+1)
               h = (HISTORY_DATA*)history_tags[1].GetFirst(it);
            else
               h = (HISTORY_DATA*)history_tags[1].GetNext(it);
            if(h)
            {
               pname.set(h->m_pname);
               tname.set(h->m_tname);
               obj.sprintf(L"pid%u-tid%u", h->pid, h->tid);
               title.sprintf(L"%s [pid:%i] %s [tid:%i] [priority:%i]", 
                  pname.s() ? pname.s() : L"", 
                  h->pid, 
                  tname.s() ? tname.s() : L"", 
                  h->tid, 
                  h->priority);
            }
         }
         else 
         {
            int xx=0;
            xx=11;
         }

         if(h)
         {
            html_fname.append(obj.s());
            html_fname.append(L".html");

            data.set(L"");
            for(k=0; k<h->m_begin; k++)
            {
               data.append(L"\"0.0\", ");
            }
            data.append(h->m_data.cpu().s());

            html.append(L"\n\
                         <script type=\"text/javascript\" charset=\"utf-8\">\n\
                         var labels = new Array(");
            html.append(slabels.s());
            html.append(L");\n\
                         var plot = new Object();\n\
                         plot.title = \"");
            html.append(title.s());
            html.append(L"\";\n\
                         plot.data = new Array(");
            html.append(data.s());
            html.append(L");\n\
                         window.onload = function ()\n\
                         {\n\
                            drawGraph(\"canvas\", plot.data, labels);\n\
                         };\n\
                         </script>\n<br/><b>\n");
			html.append(title.s());
			html.append(L"</b><br/>\n\
                        <div class=\"graph\">\n\
                            <div id=\"canvas\"></div>\n\
                         </div>\n\
                         <br/>\n\
						 ");
         }//if
         else
         {
         }
      }//	!if(j==0)//first

      tmp.set(html_dt[HTML_3], html_sz[HTML_3]);
      html.append(tmp.s(), tmp.size());

	  const char* fn = html_fname.ansi();
      fp=fopen(fn, "wt");
      if(!fp)
         goto Exit;
      fwrite(html.ansi(), html.size(), 1, fp);
      fclose(fp);

   }
   ret = true;
Exit:
   dlg.DestroyWindow();
   for(i=0; i<NUM_RES; i++)
   {
	   if(hglob[i])
		   FreeResource(hglob[i]);
   }
#endif//USE_HISTORY_DATA
   return ret;
}
