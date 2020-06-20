#include "stdafx.h"
#include "../common/types.h"
#include "../common/util.h"
extern "C" {
#include "comm.h"
}
#include "../fileio/fileio.h"
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "devicelist.h"
#include "gdgraph.h"
#include "history.h"
#include "dlgRemoteCPU.h"
#include "dialog2.h"

GDGraph::GDGraph()
: m_thread_count(0)
{
	InitializeCriticalSection(&m_qlock);
	m_sem = CreateSemaphore(0, 0, 10240, 0);
	
	memset(m_thread, 0, sizeof(m_thread));
	
	gdStart();
}

GDGraph::~GDGraph()
{
	gdStop();
	
	// Wait for all threads to terminate
	while(m_thread_count>0)
	{
		OutputDebugString(L"~GDGraph() - Wait for threads to quit\n");
		Sleep(100);
	}

	DWORD num_threads = sizeof(m_thread)/sizeof(DWORD);
	for(DWORD i=0; i<num_threads; i++)
	{
		CloseHandle(m_thread[i]);
	}
	
	CloseHandle(m_sem);
	DeleteCriticalSection(&m_qlock);
}

void GDGraph::gdStart()
{
	m_quit = false;
	for(int i=0; i<sizeof(m_thread)/sizeof(DWORD); i++)
	{
		if(m_thread[i])
			CloseHandle(m_thread[i]);

		m_thread_count++;
		m_thread[i] = CreateThread(0, 0, gdThreadProc, this, 0, 0);
	}
}

DWORD WINAPI GDGraph::gdThreadProc(LPVOID lpParameter)
{
	GDGraph *pthis = (GDGraph*)lpParameter;
	pthis->gdDeque();

	pthis->m_thread_count--;
	return 1;
}

void GDGraph::gdStop()
{
	m_quit=true;
	OutputDebugString(L"GDGraph::gdStop()\n");
}

bool GDGraph::gdEnque(STRING &file, DATA_POINT *data, int data_sz)
{
	bool ret = false;
	
	if(!m_quit)
	{
		WORK_ITEM *wi = new WORK_ITEM();
		if(wi)
		{
			memcpy(wi->m_data, data, data_sz);
			wi->m_file.set(file.s());
			wi->m_num_samples = data_sz/sizeof(DATA_POINT);

			EnterCriticalSection(&m_qlock);
			m_que.Insert(wi);
			LeaveCriticalSection(&m_qlock);
			
			int res = ReleaseSemaphore(m_sem, 1, 0);
			if(!res)
			{
				int err = GetLastError();
			}
			ret = res>0 ? true:false;
		}
	}
	return ret;
}

int GDGraph::gdRemaining()
{
	int ret;
	EnterCriticalSection(&m_qlock);
	ret = m_que.GetCount();
	LeaveCriticalSection(&m_qlock);
	return ret;
}

void GDGraph::gdDeque()
{
	ITERATOR it;
	WORK_ITEM* wi=0;
	bool que_is_empty = false;
	DWORD ret;
	
	do 
	{
		wi = 0;
		que_is_empty = false;
		
		ret = WaitForSingleObject(m_sem, 900);
		
		EnterCriticalSection(&m_qlock);
		if(!(m_que.GetCount()>0))
			que_is_empty=true;
		LeaveCriticalSection(&m_qlock);
		
		switch(ret)
		{
      case WAIT_TIMEOUT:
         {
            int x=0;
            x=1;
         }
         break;
		case WAIT_OBJECT_0:
			{
				EnterCriticalSection(&m_qlock);
				if(m_que.GetCount()>0) 
					wi = (WORK_ITEM* )m_que.GetFirst(it);
				if(wi)
					m_que.Remove(it);
				LeaveCriticalSection(&m_qlock);
				
				if(wi)
				{
					if(!m_quit)
						gdPlotGraph(wi->m_file, wi->m_data, wi->m_num_samples);
					delete wi;
				}
			}
			break;
      default:
         {
            int x=0;
            x=1;
         }
		}
	}
	while(!que_is_empty || !m_quit);
}

BITMAPINFO GDGraph::gdCreateBmp(int width, int height)
{
	BITMAPINFO bmp_info;
	
	// Configure bitmap properties
	
	ZeroMemory(&bmp_info, sizeof(BITMAPINFO));
	bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmp_info.bmiHeader.biWidth = width;
	bmp_info.bmiHeader.biHeight = height;
	bmp_info.bmiHeader.biPlanes = 1;
	bmp_info.bmiHeader.biBitCount = 32;
	bmp_info.bmiHeader.biCompression = BI_RGB;
	bmp_info.bmiHeader.biSizeImage = 0;
	bmp_info.bmiHeader.biXPelsPerMeter = 0;
	bmp_info.bmiHeader.biYPelsPerMeter = 0;
	bmp_info.bmiHeader.biClrUsed = 0;
	bmp_info.bmiHeader.biClrImportant = 0;
	return bmp_info;
}

gdImagePtr GDGraph::gdImageTrueColorAttachBuffer(int* buffer, int sx, int sy, int stride)
{
	int i;
	int height;
	int* rowptr;
	gdImagePtr im;
	im = (gdImage *) malloc (sizeof (gdImage));
	if (!im) {
		return 0;
	}
	memset (im, 0, sizeof (gdImage));
	
#if 0
	if (overflow2(sizeof (int *), sy)) {
		return 0;
	}
#endif
	
	im->tpixels = (int **) malloc (sizeof (int *) * sy);
	if (!im->tpixels) {
		free(im);
		return 0;
	}
	
	im->polyInts = 0;
	im->polyAllocated = 0;
	im->brush = 0;
	im->tile = 0;
	im->style = 0;
	
	height = sy;
	rowptr = buffer;
	if (stride < 0) {
		int startoff = (height - 1) * stride;
		rowptr = buffer - startoff;
	}
	
	i = 0;
	while (height--) {
		im->tpixels[i] = rowptr;
		rowptr += stride;
		i++;
	}
	
	im->sx = sx;
	im->sy = sy;
	im->transparent = (-1);
	im->interlace = 0;
	im->trueColor = 1;
	im->saveAlphaFlag = 0;
	im->alphaBlendingFlag = 0;
	im->thick = 1;
	im->AA = 0;
	im->cx1 = 0;
	im->cy1 = 0;
	im->cx2 = im->sx - 1;
	im->cy2 = im->sy - 1;
	return im;
}

void GDGraph::gdImageDetachBuffer(gdImagePtr im)
{
	free(im->tpixels);
	free(im);
}


bool GDGraph::gdPlotGraph(STRING &file, DATA_POINT *data, int num_samples)
{
	HDC hdc = GetDC(0);
	HDC  mem_dc;
	BITMAPINFO bmp_info;
	void* bits;
	HBITMAP bmp=0, temp=0;
	gdImagePtr im;
	int width, height, stride;
	int white, black, green, gray;
	gdFontPtr lfont;//, gfont;
	int i;
	RECT rect, *rc;
	int xlabelh = 20;//height of the x-axis label, the area under the x-axis used for writing labels
#ifdef PERF_LOG
	DWORD start = GetTickCount(), now = 0;//$
	STRING logf;
	logf.sprintf(L"history%i.log", GetCurrentThreadId());
	FILE* log = fopen(logf.ansi(), "a+t");//$
	if(!log)
	{
		int x=0;
		x=1;
	}
#endif
	
	rect.left=50;
	rect.top=50;
	rect.right=rect.left + num_samples;
	rect.bottom=rect.top + 200 + xlabelh;
	
	rc = &rect;
	
	width = rc->right - rc->left;
	height = rc->bottom - rc->top;
	
	bmp_info = gdCreateBmp(width, height);
	
	// Create memory device context
	mem_dc = CreateCompatibleDC(hdc);
	if (!mem_dc) {
		//MessageBox(NULL, L"Can't create a compatible DC!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	
	// bits points to a shared buffer of pixels
	bits = NULL;
	bmp = CreateDIBSection(mem_dc, &bmp_info, DIB_RGB_COLORS, (void**)&bits, 0, 0);
	if(!bmp)
	{
		return false;
	}
	
	// Selecting the object before doing anything allows you to use libgd
	// together with native Windows GDI.
	temp = (HBITMAP)SelectObject(mem_dc, bmp);
	if(!temp)
	{
		return false;
	}
	
	/*stride = ((width * 1 + 3) >> 2) << 2;*/
	// always uses 32bit in BMPINFO
	stride = width;
	im = NULL;
	
	// Attach shared buffer of pixels to GD image
	// Negative stride places 0,0 in upper-left corner
	im = gdImageTrueColorAttachBuffer((int*)bits, width, height, -stride);
	if (!im) {
		//MessageBox(NULL, L"GD image creation failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	// Start of GD drawing
	white = gdImageColorAllocate(im, 255, 255, 255);
	black = gdImageColorAllocate(im, 0, 0, 0);
	green = gdImageColorAllocate(im, 0, 255, 0);
	//green = gdImageColorAllocateAlpha(im, 0, 255, 0, 220);
	//greendark = gdImageColorAllocateAlpha(im, 0, 255, 0, 120);
	gray = gdImageColorAllocateAlpha(im, 128, 128, 128, 250);
	
	// Erase background with black color
	gdImageFilledRectangle(im, 0, 0, width, height, 0x000000);
	
	// horizontal lines
	for(i=0; i<height-xlabelh; i+=20)
	{
		gdImageLine(im, 0, i, width, i, gray);
	}
	gdImageLine(im, 0, height-xlabelh, width, height-xlabelh, white);//x-axis
	
	lfont = gdFontGetLarge();
	
	for(i=1; i<num_samples; i++)
	{
		int x1 = i-1;
		int y1 = (int)(data[i-1].val)*2;
		int x2 = i;
		int y2 = (int)(data[i].val)*2;
		
		gdImageLine(im, x1, height-xlabelh-y1, x2, height-xlabelh-y2, green);
		
		if(x1==0)
		{
			STRING label;
			label.sprintf(L" %02i:%02i:%02i %04i/%02i/%02i", 
				data[i].ts.wHour,
				data[i].ts.wMinute,
				data[i].ts.wSecond,
				data[i].ts.wYear,
				data[i].ts.wMonth,
				data[i].ts.wDay);
			
			gdImageLine(im, x1, 0, x1, height-xlabelh+5, gray);//vertical line
			gdImageString(im, lfont,
				x1,
				height - (lfont->h) - 1,
				(unsigned char*)label.ansi(), white);
		}
		
	}
	
	gdImageFile(im, file.ansi());
	
	// Copy drawing from memory context (shared bitmap buffer) to screen DC.
	//BitBlt(hdc, rc->left, rc->top, width, height, mem_dc, 0, 0, SRCCOPY);
	
	// Free
	gdImageColorDeallocate(im, white);
	gdImageColorDeallocate(im, black);
	gdImageColorDeallocate(im, green);
	gdImageColorDeallocate(im, gray);
	gdImageDetachBuffer(im);
	SelectObject(mem_dc, temp);
	DeleteObject(bmp);
	DeleteObject(mem_dc);
	ReleaseDC(0, hdc);
	
#ifdef PERF_LOG
	now = GetTickCount();//$
	if(log) fprintf(log, "%i\n", now - start);//$
	if(log) fclose(log);//$
#endif
	return true;
}

#include "resource.h"

enum
{
	HTML_1,
	HTML_2,
	HTML_3,
	STYLE_CSS,
	HEADERBG_PNG,
	PBG_PNG,
	LOGO_PNG,
	NUM_RES,
};

char_t RES_FILE_NAME2[NUM_RES][64] = {
	L"", L"", L"", L"style.css", L"headerbg.png", L"pbg.png", L"remotetaskmontorlogo.png"
};

bool GDGraph::gdReport(
					   CDialog2 &dlg,
					   DEVICE * device, 
					   STRING &outdir, 
					   CDlgRemoteCPU* parent,
					   LIST *history_tags,
					   DATA_LIST &cpu,
					   DATA_LIST &mem
					   )
{
	bool ret = false;
	FILE* fp;
	STRING html, tmp;
	HRSRC hrsc[NUM_RES];
	HGLOBAL hglob[NUM_RES];
	const char* html_dt[NUM_RES];
	int html_sz[NUM_RES];
	int i, j, k, m;
	int num_ps = history_tags[HISTORY_FILE_PS].GetCount();
	int num_th = history_tags[HISTORY_FILE_TH].GetCount();
	int num_canvases = num_ps + num_th;
	STRING html_fname;
	STRING obj;
	HISTORY_DATA *h;
	ITERATOR it;
	STRING pname, tname, slabels, title, data, selectbox;
	CWnd *progressWnd, *staticWnd;
	int cond = 0;
	DWORD err=0;
	
	progressWnd = dlg.GetDlgItem(IDC_PROGRESS_LOAD_DATA);
	staticWnd = dlg.GetDlgItem(IDC_STATIC_LOADING_RTM);
	
	title.set(L"   ");
	title.sprintf(L"Compiling report. Please wait...");
	staticWnd->SetWindowText(title.s());
	
	STRING sysinfo, memtotal, memavail;
	MY_SYSTEM_INFO* si = device->getSysInfo();
	const MY_MEM_INFO* memst = device->getMemStatus();
	
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
				
				if(RES_FILE_NAME2[i][0])
				{
					tmp.set(outdir.s());
					tmp.append(RES_FILE_NAME2[i]);
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
	
	//selectbox 
	for(k=0; k<HISTORY_NUM_FILES-1; k++)
	{
		bool haveSelBox = false;
		h = (HISTORY_DATA*)history_tags[k].GetFirst(it);
		for(i=0; h; i++)
		{
			PROCESS* p = device->getProcessById(h->m_pid);
			if(p)
			{
				if(k==HISTORY_FILE_PS && p->getGenReport())
				{
					pname.set(h->m_pname);
					obj.sprintf(L"pid%u", h->m_pid);
					title.sprintf(L"%s [pid:%i]", 
						pname.s() ? pname.s() : L"", h->m_pid);
					
					if(!haveSelBox)
					{
						tmp.sprintf(L"&nbsp;&nbsp;<select id=\"selbox-pid%i\" class=\"selectmult\" onchange=\"change_category_selbox(this.options[this.selectedIndex].value)\"><option value=\"0\"   >Processes</option>",
							h->m_pid);
						selectbox.append(tmp.s());
						haveSelBox = true;
					}
					tmp.sprintf(L"\t\t\t<option value=\"pid-%i\"   >%s</option>\n",
						h->m_pid, title.s());
					selectbox.append(tmp.s());
				}
				else if(k==HISTORY_FILE_TH)
				{
               THREAD *t = p->getThreadById(h->m_tid);
               if(t && t->getGenReport())
               {
					   pname.set(h->m_pname);
					   tname.set(h->m_tname);
					   obj.sprintf(L"pid%u-tid%u", h->m_pid, h->m_tid);
					   title.sprintf(L"%s [pid:%i] %s [tid:%i] [priority:%i]", 
						   pname.s() ? pname.s() : L"", 
						   h->m_pid, 
						   tname.s() ? tname.s() : L"", 
						   h->m_tid, 
						   h->m_priority);
   					
					   if(!haveSelBox)
					   {
						   tmp.sprintf(L"&nbsp;&nbsp;<select id=\"selbox-pid%i-tid%i\" class=\"selectmult\" onchange=\"change_category_selbox(this.options[this.selectedIndex].value);\"><option value=\"0\"   >Threads</option>",
							   h->m_pid, h->m_tid);
						   selectbox.append(tmp.s());
						   haveSelBox = true;
					   }
					   tmp.sprintf(L"\t\t\t<option value=\"pid-%i-tid-%i\"   >%s</option>\n",
						   h->m_pid, h->m_tid, title.s());
					   selectbox.append(tmp.s());
               }
				}
			}//if

			h = (HISTORY_DATA*)history_tags[k].GetNext(it);
		}//for
		if(haveSelBox)
		{
			selectbox.append(L"</select>");
		}
	}
	if(selectbox.size())
	{
		selectbox.prepend(L"<div class=\"p1\">Jump to ");
		selectbox.append(L"</div>");
	}

	
	for(j=0; num_canvases && j<num_canvases+1; j++)
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
		//	<div class="p1">
		
		//	Jump to:
		html.append(selectbox.s());
		//		</div>
		tmp.set(html_dt[HTML_2], html_sz[HTML_2]);
		html.append(tmp.s(), tmp.size());
		
		if(j==0)//first
		{
			html_fname.set(outdir.s());
			html_fname.append(L"index.html");
			
			html.append(L"\n\
							  <div class=\"summary\">\n\
							  Number of Processors: ");
			tmp.sprintf(TXT("%i"), si->numberOfProcessors());
			html.append(tmp.s());
			html.append(L", \n\
				Memory total: ");
			html.append(memtotal.s());
			tmp.sprintf(L"<br/>\n\
								   </div>\n\
								   <br/>\n\
								   <b>Overall CPU %%</b><br/>\n\
								   <div class=\"graph_outer_cnt\">\n\
								   <div class=\"graph_inner_cnt\" style=\"width:%ipx;\">\n\
								   ", 
								   cpu.rc()*MAX_DATA_LIST_SIZE);
			html.append(tmp.s());
			for(m=0; m<cpu.rc(); m++)
			{
				tmp.sprintf(L"<img class=\"graph_img\" src=\"total-cpu-%i.png\"/>", m);
				html.append(tmp.s());
			}
			tmp.sprintf(L"\n\
							  </div><!--graph_inner_cnt-->\n\
							  </div><!--graph_outer_cnt-->\n\
							  <b>Overall Memory %%</b><br/>\n\
							  <div class=\"graph_outer_cnt\">\n\
							  <div class=\"graph_inner_cnt\" style=\"width:%ipx;\">\n\
							  ", 
							  cpu.rc()*MAX_DATA_LIST_SIZE);
			html.append(tmp.s());
			for(m=0; m<cpu.rc(); m++)
			{
				tmp.sprintf(L"<img class=\"graph_img\" src=\"total-mem-%i.png\"/>", m);
				html.append(tmp.s());
			}
			html.append(L"\n\
							  </div><!--graph_inner_cnt-->\n\
							  </div><!--graph_outer_cnt-->\n\
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
					h = (HISTORY_DATA*)history_tags[HISTORY_FILE_PS].GetFirst(it);
				else
					h = (HISTORY_DATA*)history_tags[HISTORY_FILE_PS].GetNext(it);
				if(h)
				{
					pname.set(h->m_pname);
					obj.sprintf(L"pid%u", h->m_pid);
					title.sprintf(L"%s [pid:%i]", 
						pname.s() ? pname.s() : L"", h->m_pid);
				}
			}
			else if(j>num_ps && j<num_canvases+1)
			{
				if(j==num_ps+1)
					h = (HISTORY_DATA*)history_tags[HISTORY_FILE_TH].GetFirst(it);
				else
					h = (HISTORY_DATA*)history_tags[HISTORY_FILE_TH].GetNext(it);
				if(h)
				{
					pname.set(h->m_pname);
					tname.set(h->m_tname);
					obj.sprintf(L"pid%u-tid%u", h->m_pid, h->m_tid);
					title.sprintf(L"%s [pid:%i] %s [tid:%i] [priority:%i]", 
						pname.s() ? pname.s() : L"", 
						h->m_pid, 
						tname.s() ? tname.s() : L"", 
						h->m_tid, 
						h->m_priority);
				}
			}
			else 
			{
				int xx=0;
				xx=11;
			}
			
			if(h && h->m_pid)
			{
				PROCESS* p = device->getProcessById(h->m_pid);
				if(p && p->getGenReport())
				{
					html.append(title.s());
					tmp.sprintf(L"</b><br/>\n\
											   <br/>\n\
											   <b>CPU %%</b><br/>\n\
											   <div class=\"graph_outer_cnt\">\n\
											   <div class=\"graph_inner_cnt\" style=\"width:%ipx;\">\n\
											   ", 
											   h->m_cpu.rc()*MAX_DATA_LIST_SIZE);
					html.append(tmp.s());
					if(h->m_pid && h->m_tid)//thread
					{
                  THREAD* t = p->getThreadById(h->m_tid);
                  if(t && t->getGenReport())
                  {
						   tmp.sprintf(L"pid-%i-tid-%i.html", 
							   h->m_pid,
							   h->m_tid);
						   html_fname.append(tmp.s());
   						
						   for(int m=0; m<h->m_cpu.rc(); m++)
						   {
							   tmp.sprintf(L"<img class=\"graph_img\" src=\"cpu-pid-%i-tid-%i-%i.png\"/>\n", 
								   h->m_pid,
								   h->m_tid,
								   m);
							   html.append(tmp.s());
						   }
                  }
					}
					else if(h->m_pid)//process
					{
						tmp.sprintf(L"pid-%i.html", 
							h->m_pid);
						html_fname.append(tmp.s());
						
						for(m=0; m<h->m_cpu.rc(); m++)
						{
							tmp.sprintf(L"<img class=\"graph_img\" src=\"cpu-pid-%i-%i.png\"/>", 
								h->m_pid,
								m);
							html.append(tmp.s());
						}
						tmp.sprintf(L"\n\
										  </div><!--graph_inner_cnt-->\n\
										  </div><!--graph_outer_cnt-->\n\
										  <br/>\n\
										  <b>Memory %%</b><br/>\n\
										  <div class=\"graph_outer_cnt\">\n\
										  <div class=\"graph_inner_cnt\" style=\"width:%ipx;\">\n\
										  ", 
										  h->m_mem.rc()*MAX_DATA_LIST_SIZE);
						html.append(tmp.s());
						for(m=0; m<h->m_mem.rc(); m++)
						{
							tmp.sprintf(L"<img class=\"graph_img\" src=\"mem-pid-%i-%i.png\"/>", 
								h->m_pid,
								m);
							html.append(tmp.s());
						}
					}
					html.append(L"\n\
									  </div><!--graph_inner_cnt-->\n\
									  </div><!--graph_outer_cnt-->\n\
									  <br/>\n\
									  ");
				}//p->shouldGenReport
			}//if(h && h->m_pid)
      }//!if(j==0)//first
	  
      tmp.set(html_dt[HTML_3], html_sz[HTML_3]);
      html.append(tmp.s(), tmp.size());
	  
	  const char* fn = html_fname.ansi();
      fp=fopen(fn, "wt");
      if(fp)
	  {
		  fwrite(html.ansi(), html.size(), 1, fp);
		  fclose(fp);
	  }
	  
   }
   ret = true;
Exit:
   for(i=0; i<NUM_RES; i++)
   {
	   if(hglob[i])
		   FreeResource(hglob[i]);
   }
   return ret;
}
