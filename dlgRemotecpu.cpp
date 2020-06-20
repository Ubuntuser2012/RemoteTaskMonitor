// CDlgRemoteCPU.cpp : implementation file
//

#include "stdafx.h"
#include "afxbutton.h"
#include "minorversion.h"
#include "../common/types.h"
#include "../common/util.h"
#include "../ds/ds.h"
#include "../fileio/fileio.h"
#include "../crypt/crypt.h"
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
#include "dlgLoadingData.h"
#include "dlgRemotecpu.h"
#include "dlgWatchCfg.h"
#include "dlgSaved.h"
#include "dlgStartProcess.h"
#include "watchlist.h"
#include "smtp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define X 0
#define Y 1

#define MAX_WINDOW_WIDTH 467
#define MAX_WINDOW_HEIGHT 400
#define MAX_TIME_BEFORE_PS_DATA_STALE 100000000//1 sec = 10^9 nanoseconds

#define HOMEPAGE_URL L"https://www.osletek.com/pg/2186"
#define CEMON_THREAD_PRIORITY 100
#define LOG_DURATION_MIN 60
#define MAX_CFG_TOKENS 3

void strtime(wchar_t *dtw, int dtsz);
STRING timeAsString();

void py_notfiy_state(int state, STRING &py_msg);
void py_got_snapshot();
void py_end(CDlgRemoteCPU *mainWnd);

#pragma warning( disable : 4355 )//disable warning about passing this pointer in constructors

struct CemonComboBoxData
{
	sockaddr_in m_addr;
	DISCOVERY_PONG m_pong;
	DWORD m_timestamp;
	CemonComboBoxData(sockaddr_in &addr, DISCOVERY_PONG &pong)
		: m_addr(addr)
		, m_pong(pong)
		, m_timestamp(GetTickCount())
	{}
};
static STRING ipToString(unsigned long ip)
{
	STRING ip_string;
	in_addr addr;
	addr.S_un.S_addr = ip;
	ip_string.set(inet_ntoa(addr));
	return ip_string;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRemoteCPU dialog


CDlgRemoteCPU::CDlgRemoteCPU(CWnd* pParent /*=NULL*/)
: CDialog(CDlgRemoteCPU::IDD, pParent)
//: CPropertySheet(L"", pParent)
, m_log_duration(LOG_DURATION_MIN)
, m_t(0)
, m_xscale(10)
, mp_display_list(0)
, m_dl_index(0)
, m_red_cpu(90)
, m_py_at_startup(0)
, m_prompt_show_report(1)
, m_checkbox_memory(false)
, m_set_cemon_priority(true)
, m_save_snapshot(false)
, m_get_thread_names(false)
, m_history(this)
, m_tabs(L"test", this)
, m_connected_at(0)
, m_processing_script(false)
, m_destroying(false)
{
	//{{AFX_DATA_INIT(CDlgRemoteCPU)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	int i;
	for(i=0; i<NUM_WINDOWS_TO_RESIZE; i++)
	{
		m_gap[i][X]=-1;
		m_gap[i][Y]=-1;
		m_window_resize[i][X]=1;
		m_window_resize[i][Y]=0;
	}
	m_window_id[0]=IDC_STATIC_PROPSHEET;
	m_window_resize[0][Y]=1;

	memset(&m_con, 0, sizeof(m_con));

	int ver = _MSC_VER;
	ver = 0;

	STRING app_dir;
	GetLocalAppDataDirectory(app_dir);
	m_license.SetApplicationDirectory(app_dir);
}

CDlgRemoteCPU::~CDlgRemoteCPU()
{
}

void CDlgRemoteCPU::PathFromTokens(STRING &path, STRING *token, int num_tokens)
{
	path.set(L"");
	if(num_tokens==2)
		path.sprintf(L"%s:%s", token[0].s(), token[1].s());
}

void CDlgRemoteCPU::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgRemoteCPU)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgRemoteCPU, CDialog)
	//{{AFX_MSG_MAP(CDlgRemoteCPU)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CDlgRemoteCPU::CfgLoad()
{
	STRING line;
	STRING token[MAX_CFG_TOKENS];
	int num_tokens = MAX_CFG_TOKENS;
	FILEIO fp;
	bool bsmtp_port = false;

   STRING dir, path1;
   if (!GetLocalAppDataDirectory(dir))
   {
	   return 0;
   }

    path1.set(dir.s());
	path1.append(L"\\rtmonitor.cfg");

	if(!fp.open(path1.s(), FILEIO_MODE_READ | FILEIO_MODE_OPEN_EXISTING))
		goto Exit;
	while(fp.readln(line))
	{
		num_tokens = MAX_CFG_TOKENS;
		if(line.explode(TXT(":"), token, num_tokens) && num_tokens < MAX_CFG_TOKENS)
		{
			int s = MAX_CFG_TOKENS - num_tokens;
			if(token[s].size() && strcmpW(token[s].s(), TXT("snap_interval"))==0)
			{
				STRING err;
				SetSnapInterval(token[s+1].s(), err);
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("redcpu"))==0)
			{
				m_red_cpu = atoiW(token[s+1].s());
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("x_scale"))==0)
			{
				m_xscale = atoiW(token[s+1].s());
			}
			else if (token[s].size() && strcmpW(token[s].s(), TXT("prompt_show_report")) == 0)
			{
				m_prompt_show_report = atoiW(token[s + 1].s()) == 1;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("checkbox_memory"))==0)
			{
				m_checkbox_memory = atoiW(token[s+1].s());
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("start_ps_path"))==0)
			{
				STRING path;
				PathFromTokens(path, &token[s], num_tokens);
				CDlgStartProcess::SetPsPath(path.s());
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("watch_ps_path"))==0)
			{
				STRING path;
				PathFromTokens(path, &token[s], num_tokens);
				CDlgWatchCfg::SetPsPath(path.s());
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("watch_map_file_path"))==0)
			{
				STRING path;
				PathFromTokens(path, &token[s], num_tokens);
				CDlgWatchCfg::SetMapFilePath(path.s());
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("report_path"))==0)
			{
				STRING path;
				PathFromTokens(path, &token[s], num_tokens);
				m_history.SetDirectory(path);
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("email_to"))==0)
			{
				m_options.SetEmailTo(token[s+1].s());
				CDlgWatchCfg::SetEmail(token[s+1].s());
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("smtp_server"))==0)
			{
				m_options.SetSMTPServer(token[s+1].s());
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("smtp_port"))==0)
			{
				m_options.SetSMTPPort(token[s+1].s());
				bsmtp_port = true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("py_script"))==0)
			{
				STRING path;
				PathFromTokens(path, &token[s], num_tokens);
				m_python.SetPythonScript(path.s());
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("py_at_startup"))==0)
			{
				m_py_at_startup = atoiW(token[s+1].s())==1 ? true:false;
				m_python.SetRunPythonScriptAtStartup(m_py_at_startup);
			}
		}
		line.clear();
	}

	if(!bsmtp_port)
	{
		STRING port(L"25");
		m_options.SetSMTPPort(port.s());
	}

	fp.close();
	return 1;
Exit:
	fp.close();
	return 0;
}

BOOL CDlgRemoteCPU::CfgSave()
{
	STRING line;
	STRING token[MAX_CFG_TOKENS];
	int num_tokens = MAX_CFG_TOKENS;
	char_t data[MAX_PATH*10];
	FILEIO fp, fp2;
	bool bsnap=false;
	bool bredcpu=false;
	bool bxscale=false;
	bool bprompt_show_report = false;
	bool bcheckbox_memory=false;
	bool bstart_ps_path=false;
	bool bwatch_ps_path=false;
	bool bwatch_map_file_path=false;
	bool breport_path=false;
	bool bsmtp_server=false;
	bool bsmtp_port=false;
	bool bemail_to=false;
	bool bpy_script=false;
	bool bpy_at_startup=false;
	int blog_duration=LOG_DURATION_MIN;

   STRING dir, path1, path2;
   if (!GetLocalAppDataDirectory(dir))
   {
	   return 0;
   }
   
    path1.set(dir.s());
	path2.set(dir.s());

   path1.append(L"\\rtmonitor.cfg");
   path2.append(L"\\~rtmonitor.cfg");
   fp.open(path1.s(), FILEIO_MODE_READ | FILEIO_MODE_OPEN_EXISTING);
	if(!fp2.open(path2.s(), FILEIO_MODE_WRITE | FILEIO_MODE_CREATE_ALWAYS))
		goto Exit;

	UpdateFields(TRUE);
	m_checkbox_memory = m_processes.GetCheckShowMemory();

	while(fp.readln(line))
	{
		num_tokens = MAX_CFG_TOKENS;
		if(line.explode(TXT(":"), token, num_tokens) && num_tokens < MAX_CFG_TOKENS)
		{
			int s = MAX_CFG_TOKENS - num_tokens;
			if(token[s].size() && strcmpW(token[s].s(), TXT("snap_interval"))==0)
			{
				GetSnapInterval(data, sizeof(data)/sizeof(char_t));
				fp2.printf(TXT("snap_interval:%s\r\n"), data);
				bsnap=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("redcpu"))==0)
			{
				fp2.printf(TXT("redcpu:%i\r\n"), m_red_cpu);
				bredcpu=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("x_scale"))==0)
			{
				fp2.printf(TXT("x_scale:%i\r\n"), m_xscale);
				bxscale=true;
			}
			else if (token[s].size() && strcmpW(token[s].s(), TXT("prompt_show_report")) == 0)
			{
				fp2.printf(TXT("prompt_show_report:%i\r\n"), m_prompt_show_report);
				bprompt_show_report = true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("checkbox_memory"))==0)
			{
				fp2.printf(TXT("checkbox_memory:%i\r\n"), m_checkbox_memory);
				bcheckbox_memory=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("start_ps_path"))==0)
			{
				fp2.printf(TXT("start_ps_path:%s\r\n"), CDlgStartProcess::GetPsPath());
				bstart_ps_path=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("watch_ps_path"))==0)
			{
				fp2.printf(TXT("watch_ps_path:%s\r\n"), CDlgWatchCfg::GetPsPath());
				bwatch_ps_path=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("watch_map_file_path"))==0)
			{
				fp2.printf(TXT("watch_map_file_path:%s\r\n"), CDlgWatchCfg::GetMapFilePath());
				bwatch_map_file_path=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("report_path"))==0)
			{
				fp2.printf(TXT("report_path:%s\r\n"), m_history.GetDirectory().s());
				breport_path=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("email_to"))==0)
			{
				STRING emailto;
				m_options.GetEmailTo(emailto);
				if(emailto.size())
					fp2.printf(TXT("email_to:%s\r\n"), emailto.s());
				bemail_to=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("smtp_server"))==0)
			{
				STRING server;
				m_options.GetSMTPServer(server);
				if(server.size())
					fp2.printf(TXT("smtp_server:%s\r\n"), server.s());
				bsmtp_server=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("smtp_port"))==0)
			{
				fp2.printf(TXT("smtp_port:%i\r\n"), m_options.GetSMTPPort());
				bsmtp_port=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("py_script"))==0)
			{
				const STRING script = m_python.GetPythonScript();
				fp2.printf(TXT("py_script:%s\r\n"), script.size() ? script.s() : L"");
				bpy_script=true;
			}
			else if(token[s].size() && strcmpW(token[s].s(), TXT("py_at_startup"))==0)
			{
				fp2.printf(TXT("py_at_startup:%i\r\n"), m_python.GetRunPythonScriptAtStartup());
				bpy_at_startup=true;
			}
		}
		else if(line.size())
		{
			fp2.printf(TXT("%s\r\n"), line.s());
		}
		line.clear();
	}

	if(!bsnap)
	{
		GetSnapInterval(data, sizeof(data)/sizeof(char_t));
		fp2.printf(TXT("snap_interval:%s\r\n"), data);
	}
	if(!bredcpu)
	{
	}
	if(!bxscale)
	{
		fp2.printf(TXT("x_scale:%i\r\n"), m_xscale);
	}
	if (!bprompt_show_report)
	{
		fp2.printf(TXT("prompt_show_report:%i\r\n"), m_prompt_show_report);
	}
	if(!bcheckbox_memory)
	{
		fp2.printf(TXT("checkbox_memory:%i\r\n"), m_checkbox_memory);
	}
	if(!bstart_ps_path)
	{
		fp2.printf(TXT("start_ps_path:%s\r\n"), CDlgStartProcess::GetPsPath());
	}
	if(!bwatch_ps_path)
	{
		fp2.printf(TXT("watch_ps_path:%s\r\n"), CDlgWatchCfg::GetPsPath());
	}
	if(!bwatch_map_file_path)
	{
		fp2.printf(TXT("watch_map_file_path:%s\r\n"), CDlgWatchCfg::GetMapFilePath());
	}
	if(!breport_path)
	{
		fp2.printf(TXT("report_path:\r\n"));
	}
	if(!bemail_to)
	{
		STRING emailto;
		m_options.GetEmailTo(emailto);
		if(emailto.size())
			fp2.printf(TXT("email_to:%s\r\n"), emailto.s());
	}
	if(!bsmtp_server)
	{
		STRING server;
		m_options.GetSMTPServer(server);
		if(server.size())
			fp2.printf(TXT("smtp_server:%s\r\n"), server.s());
	}
	if(!bsmtp_port)
	{
		fp2.printf(TXT("smtp_port:%i\r\n"), m_options.GetSMTPPort());
	}
	if(!bpy_script)
	{
		const char_t *script = m_python.GetPythonScript();
		if(script && isalnum(script[0]))
			fp2.printf(TXT("py_script:%s\r\n"), script);
	}
	if(!bpy_at_startup)
	{
		fp2.printf(TXT("py_at_startup:%i\r\n"), m_python.GetRunPythonScriptAtStartup());
	}

	fp.close();
	fp2.close();
	DeleteFile(path1.s());
	MoveFile(path2.s(), path1.s());
	return 1;

Exit:
	fp.close();
	fp2.close();
	return 0;
}

void CDlgRemoteCPU::LicenceTest()
{
	STRING license = m_license.LoadLicense();
	if (license.size() > 0)
	{
		m_license.Validate(license);
	}
	if (license.size()==0 || m_license.GetLicenseType() <= LicenseType_Trial)
	{
		//if(!m_license.VerifyTrialFromRTMServer())
		if (TrialExpired() || !IsLicenseValid())
		{
			m_tabs.SetActiveTabAndLock(TABPAGE_ABOUT);
		}
	}
	SetLicenseKeyUI(license);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRemoteCPU message handlers

BOOL CDlgRemoteCPU::OnInitDialog()
{
	CDialog::OnInitDialog();

	srand((unsigned)time(NULL));

	m_dlgerr.Create(IDD_ERROR, (CWnd*)this);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	m_hIcon = AfxGetApp()->LoadIcon(IDI_MAINFRAME);
//	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//with premature flag, the prop page hwnd is created now, otherwise it is created when user goes to the page
	m_device.m_psp.dwFlags |= PSP_PREMATURE;
	m_processes.m_psp.dwFlags |= PSP_PREMATURE;
	m_cpu_graphs.m_psp.dwFlags |= PSP_PREMATURE;
	m_mem_graphs.m_psp.dwFlags |= PSP_PREMATURE;
	m_options.m_psp.dwFlags |= PSP_PREMATURE;
	m_about.m_psp.dwFlags |= PSP_PREMATURE;
	m_python.m_psp.dwFlags |= PSP_PREMATURE;

	//add the prop pages to the propsheet
	m_tabs.AddPage(&m_device);
	m_tabs.AddPage(&m_processes);
	m_tabs.AddPage(&m_cpu_graphs);
	m_tabs.AddPage(&m_mem_graphs);
	m_tabs.AddPage(&m_options);
	m_tabs.AddPage(&m_python);
	m_tabs.AddPage(&m_about);

	//set this wnd pointer
	m_device.SetUserInputHandler(this);
	m_processes.SetUserInputHandler(this);
	m_cpu_graphs.SetUserInputHandler(this);
	m_mem_graphs.SetUserInputHandler(this);
	m_options.SetUserInputHandler(this);
	m_python.SetUserInputHandler(this);
	m_about.SetUserInputHandler(this);

	//do not show the apply button
	m_tabs.m_psh.dwFlags |= PSH_NOAPPLYNOW;
	m_tabs.Create(this,
		WS_CHILD | WS_VISIBLE,
		WS_EX_CONTROLPARENT);
	m_tabs.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
	m_tabs.ModifyStyle(0, WS_TABSTOP);

	//do not show other default buttons of a prop sheet
	m_tabs.GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
	m_tabs.GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	m_tabs.GetDlgItem(IDHELP)->ShowWindow(SW_HIDE);

	LoadPlugIcons();
	SizeTabSheet();

	STRING err;
	SetSnapInterval(SNAP_INTERVAL, err);
	SetEditUrl(HOMEPAGE_URL);

	CfgLoad();

	m_List.Create(this, &m_processes);
	m_List.Size();
	//m_List.ShowWindow(SW_HIDE);
	//m_List.SetItemCountEx(20, LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

	RemmemberWindowSize();

	if (!winsock_init())
	{
		return FALSE;
	}
	m_about.GetDlgItem(IDC_STATIC_VERSION)->SetWindowText(GetRtmVersion().s());
	((CComboBox*)m_device.GetDlgItem(IDC_COMBO_CEMONS))->SetCurSel(0);

	LicenceTest();

	if (m_py_at_startup)
	{
		SetTimer(PY_AT_STARTUP, 2000, 0);
	}
	m_options.SetCheckDontPromptShowReport(m_prompt_show_report ? false : true);

	m_checkbox_memory = FALSE;//off by default no matter what it was on last shutdown (only for WinCE)
	m_processes.SetCheckShowMemory(m_checkbox_memory ? true:false);

	m_cpu_graphs.CreateSplitter([this](const PLOT_DATA *plot_item)->void {
		EndReport(plot_item);
	});
	m_mem_graphs.CreateSplitter([this](const PLOT_DATA *plot_item)->void {
		EndReport(plot_item);
	});

	GetFieldWnd(IDC_STATIC_CONNECT_MESG)->SetWindowText(
		L"Start cemon*.exe on your device, then select it below. (Make sure you have network link)");

	DiscoveryInit();
	return true;
}

void CDlgRemoteCPU::EndReport(const PLOT_DATA *plot_item)
{
	DEVICE *device = GetDevice(0);
	if (plot_item->pid)
	{
		PROCESS* p = device->getProcessById(plot_item->pid);
		if (p)
		{
			if (plot_item->tid)
			{
				THREAD *t = p->getThreadById(plot_item->tid);
				if (t)
				{
					t->setGenReport(false);
				}
			}
			else
			{
				p->setGenReport(false);
			}
		}
	}
}

void CDlgRemoteCPU::DiscoveryInit()
{
	DISCOVERY_PING broadcast_msg;
	memset(&broadcast_msg, 0, sizeof(DISCOVERY_PING));
	strcpy(broadcast_msg.program, "rtmcli");
	broadcast_msg.version = RTMON_VERSION;

	m_device_discoverer.Init(broadcast_msg);
	m_device_discoverer.Discover([this](sockaddr_in addr, DISCOVERY_PONG &pong) {
		UpdateDiscoveredServers(addr, pong);
	}, SERVER_DISCOVERY_PORT);
	SetTimer(DEVICE_DISCOVERY_TIMER_ID, 1000, 0);
}

// If you add a minimize  to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDlgRemoteCPU::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

void CDlgRemoteCPU::RemmemberWindowSize()
{
	// TODO: Add extra initialization here
	RECT rect, rect2;
	int i;
	GetWindowRect(&rect);

	for(i=0; i<NUM_WINDOWS_TO_RESIZE; i++)
	{
		CWnd *p=GetFieldWnd(m_window_id[i]);
		p->GetWindowRect(&rect2);
		m_gap[i][X]=rect.right-rect2.right;
		m_gap[i][Y]=rect.bottom-rect2.bottom;
	}
}

void CDlgRemoteCPU::OnChangedTab(int activeTab)
{
	switch(activeTab)
	{
	case TABPAGE_DEVICE:
	case TABPAGE_PROCESSES:
	case TABPAGE_CPU_GRAPHS:
	case TABPAGE_MEM_GRAPHS:
//	case TABPAGE_POWER_LEVEL_GRAPHS:
//	case TABPAGE_CORES_GRAPHS:
	case TABPAGE_OPTIONS:
	case TABPAGE_PYTHON: 
			break;
	}
	SetTimer(TAB_CHANGED_TIMER, 0, 0);
}

void CDlgRemoteCPU::AdjustChildWindows()
{
	RECT rect, rect2;
	int i;
	int new_width;
	int new_height;

	GetWindowRect(&rect);

	for(i=0; i<NUM_WINDOWS_TO_RESIZE; i++)
	{
		if(m_gap[i][X]==-1 || m_gap[i][Y]==-1)
			break;
		CWnd *p=GetFieldWnd(m_window_id[i]);
		p->GetWindowRect(&rect2);
		if(rect.right-rect2.right!=m_gap[i][X] || rect.bottom-rect2.bottom!=m_gap[i][Y])
		{
			if(m_window_resize[i][X] && m_window_resize[i][Y])
			{
				new_width= rect.right-m_gap[i][X]-rect2.left;
				new_height=rect.bottom-m_gap[i][Y]-rect2.top;
			}
			else if(m_window_resize[i][X])
			{
				new_width= rect.right-m_gap[i][X]-rect2.left;
				new_height=rect2.bottom-rect2.top;
			}
			else if(m_window_resize[i][Y])
			{
				new_width= rect2.right-rect2.left;
				new_height=rect.bottom-m_gap[i][Y]-rect2.top;
			}

			if(m_window_resize[i][X] || m_window_resize[i][Y])
			{
				ScreenToClient(&rect2);
				p->SetWindowPos(0, rect2.left, rect2.top, new_width, new_height,0);
			}
		}
	}
	SizeTabSheet();
	m_List.Size();
	m_cpu_graphs.Size();
	m_mem_graphs.Size();
	m_python.Size();
}

LRESULT CDlgRemoteCPU::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message)
	{
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *minmaxinfo=(MINMAXINFO *)lParam;
			minmaxinfo->ptMinTrackSize.x = MAX_WINDOW_WIDTH;
			minmaxinfo->ptMinTrackSize.y = MAX_WINDOW_HEIGHT;
			break;
		}
	case WM_SIZING:
		{
			RemmemberWindowSize();
			break;
		}
	case WM_SIZE:
		{
			AdjustChildWindows();
			break;
		}
	case WM_NCHITTEST:
		{//allow user to drag window by clicking anywhere on the client area of the form also
			int ret=CDialog::WindowProc(message, wParam, lParam);
			if(ret==HTCLIENT)
				ret=HTCAPTION;
			return ret;
		}
	case WM_CREATE:
		{
			break;
		}
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDlgRemoteCPU::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDlgRemoteCPU::OnOK() 
{
	// TODO: Add extra validation here

	//CDialog::OnOK();
}

void CDlgRemoteCPU::OnCancel() 
{
	// TODO: Add extra cleanup here

	CDialog::OnCancel();
}

void CDlgRemoteCPU::SetProcessingScript(bool processing_script)
{
	m_processing_script = processing_script;
	m_python.EnableRunButton(processing_script ? FALSE:TRUE);
}

int CDlgRemoteCPU::ProcessWinMessages(CWnd* wnd)
{
	int ret, ret1;
	MSG Msg;

	ret=1;

	ret1 = PeekMessage( &Msg, wnd->m_hWnd, 0, 0, PM_NOREMOVE);
	if(ret1)
	{
		GetMessage( &Msg, wnd->m_hWnd, 0, 0);
		WPARAM wparam = LOWORD(Msg.wParam);
		//STRING s;
		//s.sprintf(L"%i\n", Msg.message);
		//OutputDebugString(s.s());
		switch(Msg.message)
		{
		case WM_QUIT:
		case WM_CLOSE:
		case WM_DESTROY:
			if(wnd==this)
			{
				CfgSave();
				CloseDevices();
			}
			ret=0;
			break;
		}
		if(!::IsDialogMessage(wnd->m_hWnd, &Msg))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		ret = 1;//we processed a message
	}
	else
	{
		ret = -1;//there are no messages to process
	}
	return ret;
}

void CDlgRemoteCPU::KillTimers()
{
	KillTimer(MONITOR_DEVICE_TIMER_ID);
	KillTimer(GET_SNAPSHOT_TIMER_ID);
	KillTimer(SET_GET_MEMORY_INFO_TIMER_ID);
	KillTimer(TAB_CHANGED_TIMER);
	KillTimer(PY_AT_STARTUP);
	KillTimer(CANCEL_PS_WATCHES);
	KillTimer(CANCEL_MEM_WATCH);
	KillTimer(CANCEL_CPU_WATCH);
	KillTimer(UPDATE_TRIAL_TIME_REMAINING_TIMER_ID);
	//KillTimer(DEVICE_DISCOVERY_TIMER_ID);
}

void CDlgRemoteCPU::Disconnect()
{
	KillTimers();
	Connecting(CONNECTION_DATA_STATE__NONE, 0, 0);
	m_connected_at = 0;
	CloseDevices();
	GetFieldWnd(IDC_STATIC_CONNECT_MESG)->SetWindowText(L"Disconnected");
	SetButtonImage(IDC_BUTTON_CONN, IDB_POWER_PLUG_DISCONNECTED);
	GetFieldWnd(IDC_BUTTON_CONN)->Invalidate();
	GetFieldWnd(IDC_COMBO_CEMONS)->EnableWindow(true);
	SetProcessingScript(false);
}

void CDlgRemoteCPU::Connect(unsigned long ip, unsigned short port)
{
	Connecting(CONNECTION_DATA_STATE__CONNECTING, ip, port);

	m_con.ip = ip;
	m_con.port = port;
	m_con.state = CONNECTION_DATA_STATE__CONNECTING;
	m_con.parent = this;
	m_con.start_time = GetTickCount();
	LPTHREAD_START_ROUTINE threadProc = (LPTHREAD_START_ROUTINE)CDlgRemoteCPU::ConnectThread;
	m_con.thread = CreateThread(0, 0, threadProc, &m_con, CREATE_SUSPENDED, 0);

	if(!m_con.thread)
	{
		Connecting(CONNECTION_DATA_STATE__FAILED, ip, port);
		memset(&m_con, 0, sizeof(CONNECTION_DATA));
	}
	else
	{
		ResumeThread(m_con.thread);
	}

	SetTimer(MONITOR_DEVICE_TIMER_ID, 1000, 0);
}

void CDlgRemoteCPU::EnableDevicePageButtons(bool enable)
{
	GetFieldWnd(IDC_BUTTON_REPORT)->EnableWindow(enable);
	GetFieldWnd(IDC_BUTTON_SNAPSHOT)->EnableWindow(enable);
	GetFieldWnd(IDC_BUTTON_RESET)->EnableWindow(enable);
	GetFieldWnd(IDC_BUTTON_CPU)->EnableWindow(enable);
	GetFieldWnd(IDC_BUTTON_MEMORY)->EnableWindow(enable);
	GetFieldWnd(IDC_BUTTON_BATTERY)->EnableWindow(enable);
}

void CDlgRemoteCPU::Connecting(int state, unsigned long ip, unsigned short port)
{
	switch(state)
	{
	case CONNECTION_DATA_STATE__CONNECTING:
		SetFieldText(IDC_STATIC_CONNECT_MESG, L"Connecting");
		break;
	case CONNECTION_DATA_STATE__CONNECTED:
		{
			m_connected_at = GetTickCount();
			{
				STRING title;
				STRING ip_string = ipToString(ip);
				title.sprintf(TXT("Remote Task Monitor: %s:%i"), ip_string.s(), port);
				SetWindowText(title.s());
			}
			DEVICE *device = GetDevice(0);
			if (device)
			{
				m_get_thread_names = true;

				if (!m_processing_script)
				{
					int snap_interval = GetSnapInterval();
					StartCapture(snap_interval, false);
				}

				if (m_checkbox_memory)
				{
					SetTimer(SET_GET_MEMORY_INFO_TIMER_ID, 1000, 0);
				}
			}
			m_history.Start();
			KillTimer(DEVICE_DISCOVERY_TIMER_ID);
			SetTimer(DEVICE_DISCOVERY_TIMER_ID, 15000, 0);
			EnableDevicePageButtons(true);
			break;
		}
	case CONNECTION_DATA_STATE__NONE:
	case CONNECTION_DATA_STATE__FAILED:
		{
			SetFieldText(IDC_STATIC_CONNECT_MESG, L"DisConnected");
			{
				char_t buffer[256];
				swprintf(buffer, L"Remote Task Manager");
				SetWindowText(buffer);
			}
			STRING py_msg = OnGenerateReport(false);
			py_notfiy_state(state, py_msg);

			EnableDevicePageButtons(false);

			m_set_cemon_priority = true;
			KillTimer(DEVICE_DISCOVERY_TIMER_ID);
			SetTimer(DEVICE_DISCOVERY_TIMER_ID, 1000, 0);
			break;
		}
	}
}

void CDlgRemoteCPU::HandleConnection(ULONG ip, unsigned short port)
{
	if(m_con.thread)
	{//already connecting
		if (GetTickCount() - m_con.start_time > 5000)
		{
			TerminateThread(m_con.thread, 0);
			Connecting(CONNECTION_DATA_STATE__NONE, 0, 0);
			memset(&m_con, 0, sizeof(CONNECTION_DATA));
		}
	}
	else if(m_devices.GetCount())
	{//already connected - disconnect
		Disconnect();
	}
	else
	{
		Connect(ip, port);
	}
}

void CDlgRemoteCPU::GetSnapShot()
{
	DEVICE* device = GetDevice(0);
	if (device)
	{
		device->sendGetSnapInfo();
	}
}

void CDlgRemoteCPU::SetGetMemoryInfo()
{
	DEVICE* device = GetDevice(0);
	if (device)
	{
		device->sendGetMemoryInfo(m_checkbox_memory ? true : false);
	}
}

void CDlgRemoteCPU::SaveSnapShot()
{
	FILEIO fp;
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	wchar_t dt[64];
	char_t fname[2048];
	char_t msg[3072];

	swprintf(dt, L"[%04i-%02i-%02i--%02i-%02i-%02i-%03i]",
		sysTime.wYear,
		sysTime.wMonth,
		sysTime.wDay,
		sysTime.wHour,
		sysTime.wMinute,
		sysTime.wSecond,
		sysTime.wMilliseconds);
	swprintf(fname, L"snapshot%s.csv", dt);

	STRING path;
	if (GetLocalAppDataDirectory(path))
	{
		path.append(fname);
	}


	if(!fp.open(path.s(), FILEIO_MODE_WRITE|FILEIO_MODE_CREATE_ALWAYS))
	{
		swprintf(msg, L"Failed to create file %s", path.s());
		MessageBox(msg, L"Error", MB_ICONEXCLAMATION);
	}
	else
	{
		bool state = ((CButton*)GetFieldWnd(IDC_CHECK_SHOW_MEMORY))->GetCheck() == 1 ? true:false;   
		wchar_t memused[32];

		DEVICE *device = GetDevice(0);
		if(device)
		{
			PROCESS *p;
			THREAD *t;
			int i, j;
			const MY_MEM_INFO *mem = device->getMemStatus();
			fp.printf(L"Process\tID\tCPU %.02f\tAvg CPU\tMemory %.02f%%\tPriority\r\n", device->cpuPercent(0), device->memPercent());

			for(i=0; i<device->getProcessCount(0); i++)
			{
				fp.printf(L"\n");

				p = device->getProcess(i);

				if(state)
					swprintf(memused, L"%i", p->memoryUsed());
				else
					swprintf(memused, L"-");

				fp.printf(L"%s (%i threads)\t%i\t%.02f\t%.02f\t%s\t-\r\n", 
					p->getName(),
					p->numThreads(),
					p->getId(), 
					p->cpuPercent(0), 
					p->cpuPercent(1), 
					memused);

				for(j=0; j<p->numThreads(); j++)
				{
					t = p->getThread(j);
					fp.printf(L" %s %s\t%i\t%.02f\t%.02f\t-\t%i\r\n", 
						p->getName(),
						t->getName(), 
						t->getId(), 
						t->cpuPercent(0), 
						t->cpuPercent(1), 
						t->getPriority());
				}
			}
		}
		fp.close();

		STRING msg1;
		msg1.sprintf(L"Saved snapshot to file %s\n\nOpen file?", path.s(), 0);
		CDlgSaved dlg(msg1, path, false);
		dlg.DoModal();
	}
}

DWORD CDlgRemoteCPU::ShellExecuteThread(LPVOID lpParameter)
{
	wchar_t *path = (wchar_t*) lpParameter;

	ShellExecute(NULL, L"open", path, NULL,NULL, SW_SHOWNORMAL);

	delete [] path;

	return 1;
}

void CDlgRemoteCPU::SendEmail(const STRING &to, const STRING &subject, const STRING &body)
{
	m_options.SendEmail(to, subject, body);
}

bool CDlgRemoteCPU::OkToPrompt()
{
	return !(m_processing_script || m_destroying);
}

DWORD CDlgRemoteCPU::SendEmailThread(LPVOID lpParameter)
{
	EMAIL_DATA *edata = (EMAIL_DATA *)lpParameter;
	if(edata)
	{
		SMTP smtp;
		long status[1];
		STRING err;
		STRING to_[1];
		to_[0].set(edata->m_to.s());

		STRING ip;
		int error = 0;
		if(!GetHostByName(edata->m_server, ip, error))
		{
			if(edata->m_mainwnd->OkToPrompt())
			{
				err.sprintf(L"Could not resolve host name. Error: %i", error);
				edata->m_mainwnd->m_dlgerr.SetMessage(STRING(L"Email Error"), err, STRING(L""));
			}
		}
		else if(!smtp.SendMail(ip, edata->m_port, edata->m_from, to_, 1, status, edata->m_subject, edata->m_body, 0, 0, err))
		{
			if(edata->m_mainwnd->OkToPrompt())
			{
				edata->m_mainwnd->m_dlgerr.SetMessage(STRING(L"Email Error"), err, STRING(L""));
			}
		}

		delete edata;
	}

	return 1;
}

bool CDlgRemoteCPU::ValidateSnapInterval(int snap_interval, STRING &err)
{
	bool ret = true;
	if(!(snap_interval >= MIN_SNAP_INTERVAL && snap_interval <= MAX_SNAP_INTERVAL))
	{
		err.sprintf(L"Interval must be between %i and %i ms", MIN_SNAP_INTERVAL, MAX_SNAP_INTERVAL);
		ret = false;
	}
	return ret;
}

void CDlgRemoteCPU::StartCapture(int snap_interval, bool prompt_on_err)
{
	STRING err;
	bool is_valid = ValidateSnapInterval(snap_interval, err);
	if(is_valid)
	{
		SetTimer(GET_SNAPSHOT_TIMER_ID, snap_interval, 0);
		GetFieldWnd(IDC_EDIT_SNAP_INTERVAL)->EnableWindow(false);
		GetFieldWnd(IDC_BUTTON_CAPTURE_START)->EnableWindow(false);
		GetFieldWnd(IDC_BUTTON_CAPTURE_STOP)->EnableWindow(true);
	}
	else
	{
		if(OkToPrompt() && prompt_on_err)
		{
			MessageBox(err.s(), L"Invalid interval", MB_ICONEXCLAMATION);
		}
		KillTimer(GET_SNAPSHOT_TIMER_ID);
		GetFieldWnd(IDC_EDIT_SNAP_INTERVAL)->EnableWindow(true);
		GetFieldWnd(IDC_BUTTON_CAPTURE_START)->EnableWindow(true);
		GetFieldWnd(IDC_BUTTON_CAPTURE_STOP)->EnableWindow(false);
	}
}

void CDlgRemoteCPU::OnButtonSaveSnapshot()
{
	GetSnapShot();
	m_save_snapshot = true;
}

void CDlgRemoteCPU::OnButtonCaptureStart()
{
	int snap_interval = GetSnapInterval();
	StartCapture(snap_interval, true);
}

void CDlgRemoteCPU::OnButtonCaptureStop()
{
	StartCapture(0, false);
}

void CDlgRemoteCPU::OnButtonSetCemonPriority()
{
	for (int i = 0; i < m_devices.GetCount(); i++)
	{
		DEVICE *device = (DEVICE*)GetDevice(i);
		SetCemonPriority(device, CEMON_THREAD_PRIORITY);
	}
}

void CDlgRemoteCPU::OnCheckShowThreads() 
{
	bool state = ((CButton*)GetFieldWnd(IDC_CHECK_SHOW_THREADS))->GetCheck() == 1 ? true:false;   
	if(mp_display_list)
	{
		m_display_list[0].setShowThreads(state);
		m_display_list[1].setShowThreads(state);
	}
}

void CDlgRemoteCPU::OnCheckShowMemory(bool state)
{
	m_checkbox_memory = state;
	SetGetMemoryInfo();
}

int CDlgRemoteCPU::GetSnapInterval()
{
	char_t data[128];
	int ret = 0;

	GetSnapInterval(data, sizeof(data)/sizeof(char_t));

	ret = atoiW(data);

	return ret;
}

void CDlgRemoteCPU::LoadPlugIcons()
{
	m_plug[POWER_PLUG_DISCONNECTED] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_POWER_PLUG_DISCONNECTED));
	m_plug[POWER_PLUG_CONNECTED] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_POWER_PLUG_CONNECTED));
	m_plug[POWER_PLUG_TALKING1] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_POWER_PLUG_TALKING1));
	m_plug[POWER_PLUG_TALKING2] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_POWER_PLUG_TALKING2));
	m_plug[POWER_PLUG_TALKING3] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_POWER_PLUG_TALKING3));
	m_plug[POWER_PLUG_TALKING4] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_POWER_PLUG_TALKING4));
}

void CDlgRemoteCPU::UnLoadPlugIcons()
{
	for (int i = 0; i < NUM_PLUG_ICONS; ++i)
	{
		DeleteObject(m_plug[i]);
	}
}

DWORD CDlgRemoteCPU::ConnectThread( LPVOID lpThreadParameter)
{
	char_t errmsg[256];
	int error =0 ;
	DWORD ret =0;
	CONNECTION_DATA *con = (CONNECTION_DATA*) lpThreadParameter;
	SOCKET &so = con->so;
	so = socket_client_init();
	if(!so)
		goto Exit;
	if(!socket_client_connect2(so, con->ip, con->port, &error))
		goto Exit;
	if(!socket_nonblocking(so))
		goto Exit;
	con->state = CONNECTION_DATA_STATE__CONNECTED;
	con->thread = 0;
	con->parent->GetFieldWnd(IDC_STATIC_CONNECT_MESG)->SetWindowText(L"Connection successful");
	return ret;
Exit:
	swprintf(errmsg, L"Failed to connect. Error: %i", error);
	con->parent->GetFieldWnd(IDC_STATIC_CONNECT_MESG)->SetWindowText(errmsg);
	con->state = CONNECTION_DATA_STATE__NONE;
	con->thread = 0;
	con->parent->Connecting(CONNECTION_DATA_STATE__FAILED, con->ip, con->port);
	return ret;
}

STRING CDlgRemoteCPU::OnGenerateReport(bool restart_history)
{
	bool ret = false;
	STRING outfile;
	DEVICE *device = GetDevice(0);
	if (m_history.Started())
	{
		STRING outdir;
		ret = m_history.Stop(device, outdir, this);
		if (outdir.size())
		{
			outfile.sprintf(L"%sindex.html", outdir.s());
		}
		if (restart_history)
		{
			m_history.Start();
		}
	}

	if (OkToPrompt() && m_prompt_show_report && outfile.size() && ret)
	{
		STRING msg1;
		msg1.sprintf(L"Created file:\n\n%s\n\nOpen?", outfile.s());
		CDlgSaved dlg(msg1, outfile, true);
		dlg.DoModal();
	}
	return outfile;
}

void CDlgRemoteCPU::OnButtonResetDevice() 
{
	DEVICE* device = GetDevice(0);
	if(device)
		device->sendResetDevice();
}

void CDlgRemoteCPU::OnButtonResetServer() 
{
	DEVICE* device = GetDevice(0);
	if(device)
		device->sendResetServer();
}

void CDlgRemoteCPU::OnButtonLoadCapturedData()
{
	static char_t BASED_CODE g_szFilter[] = L"Remote Task Monitor capture files (*.rtm)|*.rtm|All Files (*.*)|*.*||";
	CFileDialog m_fileDialog(TRUE, 0, 0, OFN_ENABLESIZING|OFN_PATHMUSTEXIST|OFN_NONETWORKBUTTON, //|OFN_ALLOWMULTISELECT
		g_szFilter);

	char_t dir[MAX_DIR_PATH];
	if(GetCurrentDirectory(sizeof(dir)/sizeof(wchar_t), dir))
	{
		m_fileDialog.m_ofn.lpstrInitialDir=dir;
	}

	if(IDOK==m_fileDialog.DoModal())
	{
		DEVICE *device = DEVICE::create(0, 0, 0);
		if(device)
		{
			CDlgLoadingData dlgLoad;
			CString pathName=m_fileDialog.GetPathName();
			STRING file, outdir;

			file.set(pathName.GetBuffer(pathName.GetLength()), pathName.GetLength());
			if(dlgLoad.start(this, *device, file, m_history, outdir))
			{
				STRING outfile, msg1;
				outfile.sprintf(L"%sindex.html", outdir.s());
				msg1.sprintf(L"Created file:\n\n%s\n\nOpen?", outfile.s());
				CDlgSaved dlg(msg1, outfile, false);
				dlg.DoModal();
			}
			device->deref();
		}
	}
	else
	{
	}
}

bool CDlgRemoteCPU::OnCheckDontPromptShowReport(BOOL prompt_show_report)
{
	m_prompt_show_report = prompt_show_report == 1;
	return true;
}

bool CDlgRemoteCPU::OnCheckCaptureOnDev(BOOL capture)
{
	bool ret = false;
	DEVICE* device = GetDevice(0);
	if(device)
	{
		STRING msg;
		int ret1 = IDNO;

		if(!OkToPrompt())
		{
			ret1 = IDYES;
		}
		else
		{
			msg.sprintf(L"This will restart cemon on target device and disconnect you.\n\nContinue?");
			ret1 = MessageBox(msg.s(), L"Warning", MB_YESNO);
		}

		if(IDYES==ret1)
		{
			int snap_interval = GetSnapInterval();
			device->sendCaptureOnDev(CEMON_THREAD_PRIORITY, capture, snap_interval);
			ret = true;
		}
	}
	return ret;
}

void CDlgRemoteCPU::OnButtonKey(char_t *typed_key)
{
	int ret1 = IDYES;
	char_t msg[3072];
	DEVICE* device = GetDevice(0);
   STRING err;

	if(device && typed_key && wcslen(typed_key))
	{
		swprintf(msg, L"This will end your session.\n\n(You can connect again after validating the key)\n\nContinue?");
		ret1 = MessageBox(msg, L"Warning", MB_YESNO);

		if(IDYES==ret1)
		{
			Disconnect();
		}
	}

	if(IDYES==ret1)
	{
		if(typed_key)
		{
			STRING license(typed_key);
			if(license.size())
			{
				license.trim(L" ");
				if (m_license.Validate(license))
				{
					m_tabs.UnlockTabs();
					SetLicenseKeyUI(license);
				}
			}
		}
	}

	int key = m_license.GetLicenseType();
	if(key <= LicenseType_Trial)
	{
		LicenceTest();
	}

}

void CDlgRemoteCPU::ConnectionStateMachine()
{
	int key = m_license.GetLicenseType();
	switch (m_con.state)
	{
	case CONNECTION_DATA_STATE__NONE:
		if (key <= LicenseType_Trial)
		{
			DWORD now = GetTickCount();
			if (IsConnected() &&
				(now - m_connected_at > MAX_TRIAL_TIME_SEC * 1000))
			{
				Disconnect();
			}
		}
		break;
	case CONNECTION_DATA_STATE__CONNECTING:
		break;
	case CONNECTION_DATA_STATE__CONNECTED:
	{
		m_con.state = CONNECTION_DATA_STATE__NONE;
		DEVICE *device = DEVICE::create(m_con.ip, m_con.port, m_con.so);
		if (device)
		{
			if (!m_devices.Insert(device, false))
			{
				device->deref();
			}
			else
			{
				Connecting(CONNECTION_DATA_STATE__CONNECTED, m_con.ip, m_con.port);

				if (key <= LicenseType_Trial)
				{
					SetTimer(UPDATE_TRIAL_TIME_REMAINING_TIMER_ID, 1000, 0);
				}
			}
		}
	}
	break;
	}
}

void CDlgRemoteCPU::UpdateDisplayList(DEVICE *device)
{
	DISPLAY_LIST &oldlist = m_display_list[m_dl_index];
	m_dl_index = (m_dl_index + 1) % 2;
	m_display_list[m_dl_index].copyMembers(oldlist);
	m_display_list[m_dl_index].Destroy();

	for (int j = 0; j<device->getProcessCount(false);)
	{
		PROCESS *process = device->getProcess(j);
		{
			m_display_list[m_dl_index].Insert((PROCESS *)process->refptr());
			//m_cpuPercent += process->cpuPercent(0);

			int idx = -1;
			if (!m_cpu_graphs.HasGraph(process->getId(), idx) && m_cpu_graphs.HasGraph(process->getName(), idx))
			{
				PLOT_DATA * plot_item = new PLOT_DATA;
				plot_item->pid = process->getId();
				plot_item->tid = 0;
				m_cpu_graphs.AddGraph(device, plot_item);
			}
		}
		j++;
	}

	if (m_display_list[m_dl_index].GetCount() != 0)
	{
		mp_display_list = &m_display_list[m_dl_index];
	}
	if (mp_display_list &&
		mp_display_list->GetCount()> 0 &&
		oldlist.GetCount() != mp_display_list->GetCount())
	{
		int count = mp_display_list->GetCount();
		m_List.SetItemCountEx(count, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
		m_List.RedrawItems(0, count);
	}

	device->checkCPUWatch(device->cpuPercent(0));

	COMM_INIT_SETTINGS *init_settings = device->getInitSettings();
	if (init_settings->priority != -1)
	{
		init_settings->priority = -1;
	}
	if (init_settings->snap_interval != -1)
	{
		char_t snap_interval[32];
		swprintf(snap_interval, L"%i", init_settings->snap_interval);
		STRING err;
		SetSnapInterval(snap_interval, err);
		init_settings->snap_interval = -1;
	}
	if (init_settings->capture_on_dev != -1)
	{
		SetCaptureOnDev(init_settings->capture_on_dev ? true : false);
		init_settings->capture_on_dev = -1;
	}

	ShowSysInfo(device->cpuPercent(0), device->memPercent(), device->getSysInfo());
	UpdateGraphs(m_t, device, device->getSysInfo()->numberOfProcessors());
	UpdateDevicePageButtons(device);
	m_t++;

	if (m_get_thread_names)
	{
		m_get_thread_names = false;
		device->sendGetThreadNames();
	}
	if (m_save_snapshot)
	{
		m_save_snapshot = false;
		SaveSnapShot();
	}
	py_got_snapshot();
	if (m_history.Started())
	{
		m_history.Write(device, this);
	}
}

void CDlgRemoteCPU::UpdateDevicePageButtons(DEVICE *device)
{
	int percent;
	int img_id;
	STRING txt;

	percent = (int)device->cpuPercent(0);
	img_id = IDB_GREEN_BAR_GRAPH0 + percent / 10;
	if (device->cpuPercent(0) - percent > 0.5)
	{
		img_id++;
	}
	SetButtonImage(IDC_BUTTON_CPU, img_id);
	txt.sprintf(TXT("CPU %.02f%%"), device->cpuPercent(0));
	SetFieldText(IDC_BUTTON_CPU, (char_t*)txt.s());

	if (device->getMemStatus())
	{
		percent = (int)device->memPercent();
		img_id = IDB_PURPLE_BAR_GRAPH0 + percent / 10;
		if (device->memPercent() - percent > 0.5)
		{
			img_id++;
		}

		SetButtonImage(IDC_BUTTON_MEMORY, img_id);
		txt.sprintf(TXT("Memory %.02f%%\n%s/%s"),
			device->memPercent(),
			PROCESS::memoryUsedHumanReadable((device->getMemStatus()->totalMemKB - device->getMemStatus()->freeMemKB) * 1024).s(),
			PROCESS::memoryUsedHumanReadable((device->getMemStatus()->totalMemKB) * 1024).s());
		SetFieldText(IDC_BUTTON_MEMORY, (char_t*)txt.s());
	}

	if (device->powerSourceIsAC())
	{
		img_id = IDB_BATTERY_CHARGING;
		txt.sprintf(TXT("AC Power"));
	}
	else
	{
		percent = (int)device->powerPercent();
		img_id = IDB_BATTERY0 + percent / 10;
		if (device->powerPercent() - percent > 0.5)
		{
			img_id++;
		}
		txt.sprintf(TXT("Battery %.02f%%"), device->powerPercent());
	}
	SetButtonImage(IDC_BUTTON_BATTERY, img_id);
	SetFieldText(IDC_BUTTON_BATTERY, (char_t*)txt.s());
}

int CDlgRemoteCPU::ReadDevice(DEVICE *device, STRING &err)
{
	int ret = device->readSocket(err);
	if (ret > 0 || ret == -WSAEWOULDBLOCK)
	{
		int bytes_processed = device->consumeStreamData();

		if (bytes_processed > 0)
		{
			UpdateDisplayList(device);

			static int i = 0;
			SetButtonImage(IDC_BUTTON_CONN, IDB_POWER_PLUG_TALKING1 + (i + 1) % 4);
			GetFieldWnd(IDC_BUTTON_CONN)->Invalidate();
			i++;
		}
		else
		{
			SetButtonImage(IDC_BUTTON_CONN, IDB_POWER_PLUG_CONNECTED);
			GetFieldWnd(IDC_BUTTON_CONN)->Invalidate();
			GetFieldWnd(IDC_COMBO_CEMONS)->EnableWindow(false);
		}
	}
	return ret;
}

void CDlgRemoteCPU::ReadDevices()
{
	int ret = 0;
	ConnectionStateMachine();

	for(int i=0; i<m_devices.GetCount(); i++)
	{
		DEVICE *device = (DEVICE *)m_devices.GetAt(i);
		STRING err;
		ret = ReadDevice(device, err);
		if (ret < 0 && ret != -WSAEWOULDBLOCK)
		{
			Disconnect();
			if (OkToPrompt() && err.size())
			{
				MessageBox(err.s(), L"Error", MB_ICONEXCLAMATION);
			}
			break;
		}
		else if (ret == 0)
		{
			break;
		}
		if (m_set_cemon_priority && device->getProcessCount(false))
		{
			SetCemonPriority(device, CEMON_THREAD_PRIORITY);
		}
	}

	if (ret > 0)
	{
		m_set_cemon_priority = false;
	}
	m_List.RedrawItems(0, m_List.GetItemCount());
}


void CDlgRemoteCPU::OnTimer(UINT_PTR nIDEvent)
{
	int key = GetLicenseType();
	switch(nIDEvent)
	{
	case UPDATE_TRIAL_TIME_REMAINING_TIMER_ID:
		if(key == LicenseType_Trial)
		{
			SetLicenseKeyUI2();
		}
		break;
	case MONITOR_DEVICE_TIMER_ID:
		ReadDevices();
		break;
	case GET_SNAPSHOT_TIMER_ID:
		GetSnapShot();
		if (m_license.GetLicenseType() == LicenseType_Trial && TrialExpired() )
		{
			Disconnect();
			m_tabs.SetActiveTabAndLock(TABPAGE_ABOUT);
			SetLicenseKeyUI2();
		}
		break;
	case SET_GET_MEMORY_INFO_TIMER_ID:
		SetGetMemoryInfo();
		KillTimer(SET_GET_MEMORY_INFO_TIMER_ID);
		break;
	case TAB_CHANGED_TIMER:
		m_List.Size();
		m_cpu_graphs.Size();
		m_mem_graphs.Size();
		KillTimer(TAB_CHANGED_TIMER);
		break;
	case PY_AT_STARTUP:
		KillTimer(PY_AT_STARTUP);
		m_python.OnBnClickedButtonRunPy();
		break;
	case CANCEL_PS_WATCHES:
		{
			DEVICE *device = GetDevice(0);
			if(device)
				device->cancelPsWatches();
			KillTimer(CANCEL_PS_WATCHES);
		}
		break;
	case CANCEL_MEM_WATCH:
		{
			DEVICE *device = GetDevice(0);
			if(device)
				device->delMemWatch();
			KillTimer(CANCEL_MEM_WATCH);
		}
		break;
	case CANCEL_CPU_WATCH:
		{
			DEVICE *device = GetDevice(0);
			if(device)
				device->delCPUWatch();
			KillTimer(CANCEL_CPU_WATCH);
		}
		break;
	case DEVICE_DISCOVERY_TIMER_ID:
		m_device_discoverer.Discover([this](sockaddr_in addr, DISCOVERY_PONG &pong) {
			UpdateDiscoveredServers(addr, pong);
		}, SERVER_DISCOVERY_PORT);
		RemoveDeadServers();
		break;
	}
	CDialog::OnTimer(nIDEvent);
}

OBJECT* CDlgRemoteCPU::GetDisplayListObject(int i)
{
	//switch(m_display_list.getSortTechnique())
	{
		//default:
		if(mp_display_list)
			return (OBJECT*)mp_display_list->GetAt(i);
	}
	return 0;
}


DEVICE* CDlgRemoteCPU::GetDevice(int i) const
{
	return (DEVICE*)m_devices.GetAt(i);
}

int CDlgRemoteCPU::GetNumProcessors()
{
	DEVICE *device = (DEVICE *)m_devices.GetAt(0);
	if(device)
	{
		return device->getSysInfo()->numberOfProcessors();
	}
	return 0;
}

void CDlgRemoteCPU::SortBy(SORT_BY sortTechnique)
{
	if(mp_display_list)
	{
		mp_display_list->setSortTechnique(sortTechnique);
		mp_display_list->Destroy();
	}
}

void CDlgRemoteCPU::CloseDevices()
{
	if(mp_display_list)
	{
		mp_display_list->Destroy();
		mp_display_list = 0;
	}
	DEVICE *device = (DEVICE *)m_devices.GetAt(0);
	if(device)
	{
		device->sendDisconnect();      
		device->close();
		device->deref();
		m_devices.Remove(0);
	}
}

void CDlgRemoteCPU::OnDestroy() 
{
	m_destroying = true;
	CfgSave();
#if USE_TRIAL
	m_license.TrialWrite(true);
#endif
	//CloseDevices();
	KillTimer(DEVICE_DISCOVERY_TIMER_ID);
	Disconnect();
	UnLoadPlugIcons();
	CDialog::OnDestroy();
}
void CDlgRemoteCPU::ShowSysInfo(float cpu, float memLoad, const MY_SYSTEM_INFO *sysinfo)
{
	DEVICE* device = GetDevice(0);

	int num_cores = device->numCores();
	STRING msg;
	msg.sprintf(TXT("%s %s, %i Core%c"), device->processArchitecture().s(), device->clockSpeed().s(), num_cores, num_cores > 1 ? L's': L' ');
	GetFieldWnd(IDC_STATIC_PROCESSOR_INFO)->SetWindowText(msg.s());

	STRING buffer;

	buffer.sprintf(L"CPU %0.02f%%", cpu);
	m_List.SetColumnTitle(LIST_COLUMN_CPU, (char_t*)buffer.s());
	if(device)
	{
		if(!m_display_list[0].getShowThreads())
			buffer.sprintf(L"Process (%i)", device->getNumProcesses());
		else
			buffer.sprintf(L"Process (%i)(%i threads)", device->getNumProcesses(), device->getNumThreads());
		m_List.SetColumnTitle(LIST_COLUMN_PROCESS, (char_t*)buffer.s());
	}
	buffer.sprintf(L"Memory %u%%", memLoad);
	m_List.SetColumnTitle(LIST_COLUMN_MEMORY, (char_t*)buffer.s());

	if(MY_OS_LINUX == sysinfo->os)
	{
		m_checkbox_memory = TRUE;//off by default no matter what it was on last shutdown (only for WinCE)
		m_processes.SetCheckShowMemory(true);
		GetFieldWnd(IDC_CHECK_SHOW_MEMORY)->ShowWindow(SW_HIDE);
		m_List.EnableMenuItem(ID_THREAD_PRIORITY, FALSE);//set thread priority
		m_List.EnableMenuItem(ID_THREAD_KILL, FALSE);//set thread priority
		m_List.EnableMenuItem(ID_THREAD_SUSPEND, FALSE);//set thread priority
		m_List.EnableMenuItem(ID_THREAD_RESUME, FALSE);//set thread priority
		m_List.EnableMenuItem(ID_CRASH_WATCH, FALSE);
		EnableField(IDC_BUTTON_RESET_SERVER, FALSE);
		//todo disable menu items
	}
}

void CDlgRemoteCPU::SetButtonsState(int state)
{
	int key = m_license.GetLicenseType();
	EnableField(IDC_BUTTON_RESET_DEVICE, state);
	EnableField(IDC_BUTTON_RESET_SERVER, state);
	if(key > LicenseType_Trial)
	{
		EnableField(IDC_BUTTON_LOAD_CAPTURED_DATA, !state);
		EnableField(IDC_CHECK_CAPTURE_ON_DEV, state);
	}
	EnableField(IDC_BUTTON_SAVE_SNAPSHOT, state);
	EnableField(IDC_BUTTON_CAPTURE_START, state);
	EnableField(IDC_BUTTON_CAPTURE_STOP, state);
	EnableField(IDC_EDIT_SNAP_INTERVAL, state);
}

void CDlgRemoteCPU::SetLicenseKeyUI(const STRING &key)
{
	if(key.size())
	{
		GetFieldWnd(IDC_EDIT_KEY)->SetWindowText(key.s());
		GetFieldWnd(IDC_EDIT_KEY)->UpdateData(FALSE);
		//GetFieldWnd(IDC_EDIT_KEY)->EnableWindow(FALSE);
		//GetFieldWnd(IDC_BUTTON_KEY)->EnableWindow(FALSE);

		STRING msg;
		if (m_license.GetBuyerEmail().size() > 0)
		{
			msg.sprintf(L"Licensed to %s", m_license.GetBuyerEmail().s());
		}
		GetFieldWnd(IDC_STATIC_KEYSTR)->SetWindowText(msg.s());
		GetFieldWnd(IDC_STATIC_KEYSTR)->UpdateData(FALSE);
	}

	SetLicenseKeyUI2();
	ToggleFeatures();
}

void CDlgRemoteCPU::SetTrialUIMessage(STRING &msg, int msRemaining, bool connected)
{
	if(msRemaining < 0)
	{
		msg.set(L"Trial expired - Please purchase a license to continue using Remote Task Monitor");
		GetFieldWnd(IDC_BUTTON_PURCHASE_LICENSE)->ShowWindow(SW_SHOW);
	}
	else
	{
		int secRemaing = msRemaining/1000;
		if(connected)
		{
			msg.sprintf(L"RTM Trial will disconnect from device in %i:%02i minutes", 
				secRemaing/60, secRemaing%60);
		}
		else
		{
			msg.sprintf(L"RTM Trial expires in %i:%02i minutes", 
				secRemaing/60, secRemaing%60);
		}
		GetFieldWnd(IDC_BUTTON_PURCHASE_LICENSE)->ShowWindow(SW_HIDE);
	}
}

void CDlgRemoteCPU::SetLicenseKeyUI2()
{
	STRING msg(L" ");
	LicenseType license_type = m_license.GetLicenseType();

	switch(license_type)
	{
	case LicenseType_Invalid:
		{
			GetFieldWnd(IDC_BUTTON_START_TRIAL)->ShowWindow(SW_SHOW);
			GetFieldWnd(IDC_BUTTON_PURCHASE_LICENSE)->ShowWindow(SW_HIDE);
			GetFieldWnd(IDC_BUTTON_VALIDATE_KEY)->ShowWindow(SW_HIDE);
			msg.sprintf(L"Click the 'Start Trial' button to get your trial key");
		}
		break;
	case LicenseType_Trial:
		if(!m_license.IsLicenseValid())
		{
			SetTrialUIMessage(msg, -1, false);
		}
		else
		{
			int msRemaining;
			if(IsConnected())
			{
				DWORD now = GetTickCount();
				int timeSpent = now - m_connected_at;
				msRemaining = MAX_TRIAL_TIME_SEC*1000 - timeSpent;
			}
			else
			{
				msRemaining = (MAX_TRIAL_TIME_SEC - m_license.TrialSecondsElapsedSinceStart()) * 1000;
			}
			SetTrialUIMessage(msg, msRemaining, m_connected_at > 0);
		}

		GetFieldWnd(IDC_BUTTON_START_TRIAL)->ShowWindow(SW_HIDE);
		GetFieldWnd(IDC_BUTTON_VALIDATE_KEY)->ShowWindow(SW_HIDE);
		break;
	case LicenseType_Master:
		{
			GetFieldWnd(IDC_BUTTON_START_TRIAL)->ShowWindow(SW_HIDE);
			GetFieldWnd(IDC_BUTTON_PURCHASE_LICENSE)->ShowWindow(SW_SHOW);
			GetFieldWnd(IDC_BUTTON_PURCHASE_LICENSE)->SetWindowText(TXT("Upgrade License"));
			GetFieldWnd(IDC_BUTTON_VALIDATE_KEY)->ShowWindow(SW_SHOW);

			EnableField(IDC_BUTTON_START_TRIAL, false);
			EnableField(IDC_BUTTON_PURCHASE_LICENSE, false);
			GetFieldWnd(IDC_BUTTON_START_TRIAL)->ShowWindow(SW_HIDE);
			GetFieldWnd(IDC_BUTTON_PURCHASE_LICENSE)->ShowWindow(SW_HIDE);
			GetFieldWnd(IDC_BUTTON_VALIDATE_KEY)->ShowWindow(SW_SHOW);
		}
		break;
	}
	if(msg.size())
	{
		GetFieldWnd(IDC_STATIC_KEYSTR2)->SetWindowText(msg.s());
	}
}

void CDlgRemoteCPU::SetCemonPriority(DEVICE *device, int priority)
{
	if(device)
	{
		int i,j;
		PROCESS *p;
		THREAD *t;

		for(i=0; i<device->getProcessCount(0); i++)
		{
			p = device->getProcess(i);
			if(!p)
				continue;
			if(!p->getName())
				continue;
			STRING pname(p->getName());
			STRING arr[3];
			int num_tokens=3;
			pname.explode(L".", arr, num_tokens);
			if(num_tokens>0 && strcmpW(arr[0].s(), L"cemon")==0)
			{
				for(j=0; j<p->numThreads(); j++)
				{
					t = p->getThread(j);
					if(t)
					{
						device->sendSetThreadPriority(t->getId(), priority);
					}
				}
			}
		}
	}
}

bool CDlgRemoteCPU::SetSnapInterval(const char_t *snap_interval, STRING &err)
{
	int interval = atoiW(snap_interval);
	bool is_valid = ValidateSnapInterval(interval, err);
	if(is_valid)
	{
		m_options.m_edit_snap_interval.SetWindowText(snap_interval);
		m_options.m_edit_snap_interval.UpdateData(FALSE);
	}
	return is_valid;
}

void CDlgRemoteCPU::SetCaptureOnDev(bool enabled)
{
	m_options.SetCheckCaptureOnDev(enabled);
	m_options.m_edit_snap_interval.UpdateData(FALSE);
}

void CDlgRemoteCPU::SetEditUrl(const char_t *val)
{
	m_about.m_edit_url1.SetWindowText(val);
	m_about.m_edit_url1.UpdateData(FALSE);
}

void CDlgRemoteCPU::GetSnapInterval(char_t *data, int len)
{
	m_options.m_edit_snap_interval.UpdateData(TRUE);
	m_options.m_edit_snap_interval.GetWindowText(data, len);
}

void CDlgRemoteCPU::GetEditUrl(char_t *data, int len)
{
	m_about.m_edit_url1.UpdateData(TRUE);
	m_about.m_edit_url1.GetWindowText(data, len);
}

void CDlgRemoteCPU::EnableField(int fieldID, BOOL bEnable)
{
	switch(fieldID)
	{
	case IDC_BUTTON_SNAPSHOT:
	case IDC_BUTTON_REPORT:
	case IDC_BUTTON_RESET:
	case IDC_BUTTON_CPU:
	case IDC_BUTTON_MEMORY:
	case IDC_BUTTON_BATTERY:
		m_device.GetDlgItem(fieldID)->EnableWindow(bEnable);
		break;
	case IDC_BUTTON_CAPTURE_START:
	case IDC_BUTTON_CAPTURE_STOP:
	case IDC_EDIT_SNAP_INTERVAL:
	case IDC_CHECK_CAPTURE_ON_DEV:
		m_options.GetDlgItem(fieldID)->EnableWindow(bEnable);
		break;
	case IDC_BUTTON_LOAD_CAPTURED_DATA:
		m_options.GetDlgItem(fieldID)->EnableWindow(bEnable);
		break;
	case IDC_BUTTON_START_TRIAL:
	case IDC_BUTTON_PURCHASE_LICENSE:
	case IDC_BUTTON_VALIDATE_KEY:
		m_about.GetDlgItem(fieldID)->EnableWindow(bEnable);
		break;
	}
}

CWnd* CDlgRemoteCPU::GetFieldWnd(int fieldID)
{
	switch(fieldID)
	{
	case IDC_BUTTON_SNAPSHOT:
	case IDC_BUTTON_REPORT:
	case IDC_BUTTON_RESET:
	case IDC_BUTTON_CPU:
	case IDC_BUTTON_MEMORY:
	case IDC_BUTTON_BATTERY:
	case IDC_BUTTON_CONN:
	case IDC_STATIC_CONNECT_MESG:
	case IDC_STATIC_SYSTEM_INFO:
	case IDC_STATIC_MEMORY_INFO:
	case IDC_STATIC_PROCESSOR_INFO:
	case IDC_COMBO_CEMONS:
		return m_device.GetDlgItem(fieldID);
	case IDC_EDIT_KEY:
	case IDC_STATIC_KEYSTR:
	case IDC_STATIC_KEYSTR2:
	case IDC_BUTTON_KEY:
	case IDC_BUTTON_START_TRIAL:
	case IDC_BUTTON_PURCHASE_LICENSE:
	case IDC_BUTTON_VALIDATE_KEY:
		return m_about.GetDlgItem(fieldID);
	case IDC_BUTTON_SAVE_SNAPSHOT:
	case IDC_BUTTON_CAPTURE_START:
	case IDC_BUTTON_CAPTURE_STOP:
	case IDC_EDIT_SNAP_INTERVAL:
	case IDC_CHECK_CAPTURE_ON_DEV:
	case IDC_BUTTON_LOAD_CAPTURED_DATA:
	case IDC_EDIT_SMTP_SERVER:
	case IDC_EDIT_SMTP_PORT:
	case IDC_EDIT_EMAILTO:
	case IDC_BUTTON_EMAIL_TEST:
	case IDC_CHECK_DONT_PROMPT_SHOW_REPORT:
		return m_options.GetDlgItem(fieldID);
	case IDC_CHECK_SHOW_THREADS:
	case IDC_CHECK_SHOW_MEMORY:
	case IDC_STATIC_MONITOR_PS:
		return m_processes.GetDlgItem(fieldID);
	case IDC_STATIC_MONITOR:
		return m_cpu_graphs.GetDlgItem(fieldID);
	case IDC_STATIC_PROPSHEET:
		return GetDlgItem(fieldID);
	}
	return 0;
}

void CDlgRemoteCPU::SetFieldText(int fieldID, char_t *val)
{
	switch(fieldID)
	{
	case IDC_BUTTON_SNAPSHOT:
	case IDC_BUTTON_REPORT:
	case IDC_BUTTON_RESET:
	case IDC_BUTTON_CPU:
	case IDC_BUTTON_MEMORY:
	case IDC_BUTTON_BATTERY:
		m_device.GetDlgItem(fieldID)->SetWindowText(val);
		break;
	}
}

void CDlgRemoteCPU::SetButtonImage(int fieldID, int img_id)
{
	switch (fieldID)
	{
	case IDC_BUTTON_SNAPSHOT:
	case IDC_BUTTON_REPORT:
	case IDC_BUTTON_RESET:
	case IDC_BUTTON_CPU:
	case IDC_BUTTON_MEMORY:
	case IDC_BUTTON_BATTERY:
	case IDC_BUTTON_CONN:
		{
			CMFCButton *button = (CMFCButton *)m_device.GetDlgItem(fieldID);
			button->SetImage(img_id, img_id);
		}
		break;
	}
}

void CDlgRemoteCPU::UpdateFields(BOOL bSaveAndValidate)
{
	m_options.m_edit_snap_interval.UpdateData(bSaveAndValidate);
	m_about.m_edit_url1.UpdateData(bSaveAndValidate);
}

void CDlgRemoteCPU::SizeTabSheet()
{
	CRect rcSheet;
	GetDlgItem( IDC_STATIC_PROPSHEET )->GetWindowRect( &rcSheet );
	ScreenToClient( &rcSheet );

	rcSheet.DeflateRect(0, 0, 0, 0);
	m_tabs.SetWindowPos(0, rcSheet.left, rcSheet.top, rcSheet.right, rcSheet.bottom, 
		SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE );

	rcSheet.DeflateRect(0, 0, 25, 25);
	m_tabs.GetTabControl()->SetWindowPos(0, rcSheet.left, rcSheet.top, rcSheet.right, rcSheet.bottom, 
		SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE );

	rcSheet.DeflateRect(1, 25, 10, 30); // instead 20 you can calculate it from tab height
	CPropertyPage* pPage1 = (CPropertyPage*)m_tabs.GetActivePage();
	pPage1->SetWindowPos(0, rcSheet.left, rcSheet.top, rcSheet.right, rcSheet.bottom, 
		SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE );

}

void CDlgRemoteCPU::AddCpuGraph(PLOT_DATA *plot_item)
{
	m_cpu_graphs.AddGraph(GetDevice(0), plot_item);
	m_tabs.SetActivePage(TABPAGE_CPU_GRAPHS);
}

void CDlgRemoteCPU::AddMemGraph(PLOT_DATA *plot_item)
{
	m_mem_graphs.AddGraph(GetDevice(0), plot_item);
	m_tabs.SetActivePage(TABPAGE_MEM_GRAPHS);
}

void CDlgRemoteCPU::ToggleFeatures()
{
	//First disable all features

	//IDD_PROCESS
	GetFieldWnd(IDC_CHECK_SHOW_THREADS)->EnableWindow(FALSE);//show threads , 
	GetFieldWnd(IDC_CHECK_SHOW_MEMORY)->EnableWindow(FALSE);//show mem, 
	//IDR_MENU_PROCESS
	m_List.EnableMenuItem(ID_THREAD_KILL, FALSE);//kill thread, 
	m_List.EnableMenuItem(ID_THREAD_SUSPEND, FALSE);//suspend thread, 
	m_List.EnableMenuItem(ID_THREAD_RESUME, FALSE);//resume thread, 
	m_List.EnableMenuItem(ID_THREAD_PRIORITY, FALSE);//set thread priority
	m_List.EnableMenuItem(ID_PLOT_CPU, FALSE);//add to plot
	m_List.EnableMenuItem(ID_PLOT_MEM, FALSE);//add to plot
	m_List.EnableMenuItem(ID_CRASH_WATCH, FALSE);
	m_List.EnableMenuItem(ID_CPU_WATCH, FALSE);
	m_List.EnableMenuItem(ID_TOTAL_CPU_WATCH, FALSE);
	m_List.EnableMenuItem(ID_MEMORY_WATCH, FALSE);
	//Options tab
	GetFieldWnd(IDC_CHECK_CAPTURE_ON_DEV)->EnableWindow(FALSE);
	GetFieldWnd(IDC_BUTTON_LOAD_CAPTURED_DATA)->EnableWindow(FALSE);
	GetFieldWnd(IDC_CHECK_DONT_PROMPT_SHOW_REPORT)->EnableWindow(FALSE);
	GetFieldWnd(IDC_EDIT_SMTP_SERVER)->EnableWindow(FALSE);
	GetFieldWnd(IDC_EDIT_SMTP_PORT)->EnableWindow(FALSE);
	GetFieldWnd(IDC_EDIT_EMAILTO)->EnableWindow(FALSE);
	GetFieldWnd(IDC_BUTTON_EMAIL_TEST)->EnableWindow(FALSE);

	//Next enable features

	//IDR_MENU_PROCESS
	m_List.EnableMenuItem(ID_CRASH_WATCH, TRUE);
	m_List.EnableMenuItem(ID_CPU_WATCH, TRUE);
	m_List.EnableMenuItem(ID_TOTAL_CPU_WATCH, TRUE);
	m_List.EnableMenuItem(ID_MEMORY_WATCH, TRUE);
	//Options tab
	GetFieldWnd(IDC_EDIT_SMTP_SERVER)->EnableWindow(TRUE);
	GetFieldWnd(IDC_EDIT_SMTP_PORT)->EnableWindow(TRUE);
	GetFieldWnd(IDC_EDIT_EMAILTO)->EnableWindow(TRUE);
	GetFieldWnd(IDC_BUTTON_EMAIL_TEST)->EnableWindow(TRUE);

	//IDD_PROCESS
	GetFieldWnd(IDC_CHECK_SHOW_THREADS)->EnableWindow(TRUE);//show threads , 
	GetFieldWnd(IDC_CHECK_SHOW_MEMORY)->EnableWindow(TRUE);//show mem, 
	//IDR_MENU_PROCESS
	m_List.EnableMenuItem(ID_THREAD_KILL, TRUE);//kill thread, 
	m_List.EnableMenuItem(ID_THREAD_SUSPEND, TRUE);//suspend thread, 
	m_List.EnableMenuItem(ID_THREAD_RESUME, TRUE);//resume thread, 
	m_List.EnableMenuItem(ID_THREAD_PRIORITY, TRUE);//set thread priority
	m_List.EnableMenuItem(ID_PLOT_CPU, TRUE);//add to plot
	m_List.EnableMenuItem(ID_PLOT_MEM, TRUE);//add to plot
	//Options tab
	GetFieldWnd(IDC_CHECK_DONT_PROMPT_SHOW_REPORT)->EnableWindow(TRUE);
	//IDR_MENU_PROCESS
	m_List.EnableMenuItem(ID_PLOT_CPU, TRUE);//add to plot
	m_List.EnableMenuItem(ID_PLOT_MEM, TRUE);//add to plot
	//Options tab
	GetFieldWnd(IDC_CHECK_CAPTURE_ON_DEV)->EnableWindow(TRUE);
	GetFieldWnd(IDC_BUTTON_LOAD_CAPTURED_DATA)->EnableWindow(TRUE);

	int key = m_license.GetLicenseType();
	if(key <= LicenseType_Trial)
	{
		//python at startup is disable in trial  
		m_py_at_startup = false;
		m_python.EnableCheckAutoStart(false);
		m_python.EnableRunButton(false);
	}
	else
	{
		m_python.EnableCheckAutoStart(true);
		m_python.EnableRunButton(true);
	}
}

bool CDlgRemoteCPU::GetMyCurrentDirectory(STRING &sdir) const
{
	static STRING _sdir;
	if (!_sdir.size())
	{
		char_t dir[MAX_DIR_PATH];
		if (!GetCurrentDirectory(sizeof(dir) / sizeof(char_t), dir))
		{
			goto Exit;
		}
		_sdir.set(dir);
	}
	sdir.set(_sdir.s());
	return true;
Exit:
	return false;
}

bool CDlgRemoteCPU::GetLocalAppDataDirectory(STRING &dir) const
{
	dir.clear();

	LPWCH lpvEnv = GetEnvironmentStrings();
	for (LPTSTR lpszVariable = (LPTSTR)lpvEnv; *lpszVariable; lpszVariable++)
	{
		STRING var;
		while (*lpszVariable)
		{
			var.append(*lpszVariable++);
		}
		var.append(TXT('\0'));
		if (memcmp(TXT("LOCALAPPDATA"), var.s(), strlenW(TXT("LOCALAPPDATA"))*sizeof(char_t))==0)
		{
			var.substr(dir, strlenW(TXT("LOCALAPPDATA=")));
			dir.append(L"\\Remote Task Monitor");
			FILEIO::mkdir(dir);
			break;
		}
	}
	FreeEnvironmentStrings(lpvEnv);
	return true;
}

bool CDlgRemoteCPU::GetHostByName(STRING &domain_name, STRING &ip, int &error)
{
	bool ret = false;
	struct hostent* host = 0;

	ip.clear();
	error = 0;

	const char *domain = domain_name.ansi();
	host = gethostbyname(domain);
	if(!host)
	{
		error=WSAGetLastError();
	}
	else if(!host->h_addr_list || !*host->h_addr_list[0])
	{
	}
	else 
	{//use the first IP assigned to this hostname
		ip.sprintf(L"%u.%u.%u.%u", 
			(unsigned char)host->h_addr_list[0][0],
			(unsigned char)host->h_addr_list[0][1],
			(unsigned char)host->h_addr_list[0][2],
			(unsigned char)host->h_addr_list[0][3]
		);
		ret = true;
	}
	return ret;
}

float CDlgRemoteCPU::GetTotalCpu() const
{
	DEVICE *device = GetDevice(0);
	return device ? device->cpuPercent(0) : 0;
}

void CDlgRemoteCPU::CreateGraphs(int num_processors)
{
	if (!m_cpu_graphs.GetGraphId())
	{
		m_cpu_graphs.CreateGraph(m_xscale, 0x0000ff00, num_processors);
	}
	if (!m_mem_graphs.GetGraphId())
	{
		m_mem_graphs.CreateGraph(m_xscale, 0x0000ff00);
	}
}

void CDlgRemoteCPU::UpdateGraphs(unsigned int x, const DEVICE *device, int num_processors)
{
	CreateGraphs(num_processors);

	m_cpu_graphs.UpdateGraphs(x, device);
	m_mem_graphs.UpdateGraphs(x, device);
}

STRING CDlgRemoteCPU::GetRtmVersion() const
{
	STRING version;
	version.sprintf(TXT("Version %.02f%c"), RTMON_VERSION / 100.0f, RTMON_VERSION_MINOR);
	return version;
}

bool CDlgRemoteCPU::IsConnected() const
{
	return m_connected_at > 0 && m_devices.GetCount() > 0;
}

struct CONNECTED_DEVICE : public OBJECT
{
	in_addr addr;
	__int64 server;
	DWORD last_update_at;
};

int CDlgRemoteCPU::GetNumDiscoveredDevices() const
{
	return m_discovered_devices.GetCount();
}

void CDlgRemoteCPU::ConnectToDiscoveredDevice(ULONG ip, DISCOVERY_PONG &pong)
{
	if (pong.version != RTMON_VERSION ||
		strcmp(pong.program, "rtmsrv") != 0)
	{
		logg(TXT("server version %i mismatch, expecting %i"), pong.version, RTMON_VERSION);
		return;
	}
	if (GetLicenseType() <= LicenseType_Trial &&  TrialExpired())
	{
		logg(TXT("trial expired"));
		return;
	}
	if (!IsLicenseValid())
	{
		logg(TXT("license invalid"));
		return;
	}
	if (!IsLicenseValid())
	{
		logg(TXT("using trial license %i"), GetLicenseType());
		return;
	}
	if(0)
	{
		STRING msg;
		STRING ip_string = ipToString(ip);
		msg.sprintf(TXT("[%s] recvd discovery packet from device %s"),
			timeAsString().s(), ip_string.s());
		logg(TXT("%s\n"), msg.s());
	}
	int i = 0;
	ITERATOR it;
	for (CONNECTED_DEVICE* device = (CONNECTED_DEVICE*)m_discovered_devices.GetFirst(it);
		i < m_discovered_devices.GetCount();
		device = (CONNECTED_DEVICE*)m_discovered_devices.GetNext(it), i++)
	{
		if(device->addr.S_un.S_addr != ip)
		{
			if (device->server == pong.instance)
			{
				break;
			}
			else
			{
				continue;
			}
		}
		if (GetTickCount() - device->last_update_at > 20000)
		{
			STRING ip_string = ipToString(ip);
			STRING msg;
			msg.sprintf(TXT("removing device after %i ms with IP %s"), GetTickCount() - device->last_update_at, ip_string.s());
			logg(TXT("%s\n"), msg.s());

			m_discovered_devices.Remove(it);
			delete device;
		}
		else
		{
			device->last_update_at = GetTickCount();
		}
		break;
	}
	if (i == m_discovered_devices.GetCount())
	{
		STRING ip_string = ipToString(ip);
		STRING msg;
		msg.sprintf(TXT("adding new device from IP %s"), ip_string.s());
		logg(TXT("%s\n"), msg.s());

		CONNECTED_DEVICE *new_device = new CONNECTED_DEVICE;
		new_device->server = pong.instance;
		new_device->addr.S_un.S_addr = ip;
		new_device->last_update_at = GetTickCount();
		m_discovered_devices.Insert(new_device);
	}

	if (m_discovered_devices.GetCount() > 1)
	{
		m_tabs.SetActiveTabAndLock(TABPAGE_ABOUT);
		GetFieldWnd(IDC_STATIC_KEYSTR2)->SetWindowText(L"Multiple devices were detected. The license only allows monitoring one device.\n\n"
		L"Please turn off other devices and restart Remote Task Monitor");
	}
	else if(!IsConnected())
	{
		HandleConnection(ip, pong.server_port);
	}
}

void CDlgRemoteCPU::UpdateDiscoveredServers(sockaddr_in addr, DISCOVERY_PONG &pong)
{
	CComboBox * combo_box_p = (CComboBox*)m_device.GetDlgItem(IDC_COMBO_CEMONS);
	if (combo_box_p && combo_box_p->FindString(-1, pong.name) == CB_ERR)
	{
		STRING s(pong.name);
		STRING ip_string;
		ip_string.set(inet_ntoa(addr.sin_addr));
		if (wcslen(pong.name) > 0)
		{
			s.sprintf(L"%s/%s", pong.name, ip_string.s());
		}
		else
		{
			s.set(ip_string.s());
		}
		int i = combo_box_p->AddString(s.s());
		combo_box_p->SetItemData(i, (DWORD_PTR)(new CemonComboBoxData(addr, pong)));
	}
//	ConnectToDiscoveredDevice(addr.sin_addr.S_un.S_addr, pong); //TODO, on select an entry in the combo box, do connect
}

void CDlgRemoteCPU::RemoveDeadServers()
{
	CComboBox * combo_box_p = (CComboBox*)m_device.GetDlgItem(IDC_COMBO_CEMONS);
	for (int i = 0; i < combo_box_p->GetCount();)
	{
		CemonComboBoxData *data = (CemonComboBoxData*)combo_box_p->GetItemData(i);
		if (data && (GetTickCount() - data->m_timestamp > 10000))
		{
			combo_box_p->DeleteString(i);
			delete data;
			continue;
		}
		++i;
	}
}

void CDlgRemoteCPU::OnConnectButtonClicked()
{
	if (IsConnected())
	{
		Disconnect();
	}
	else
	{
		CComboBox * combo_box_p = (CComboBox*)m_device.GetDlgItem(IDC_COMBO_CEMONS);
		for (int i = 0; i < combo_box_p->GetCount(); ++i)
		{
			CemonComboBoxData *data = (CemonComboBoxData*)combo_box_p->GetItemData(i);
			if (data && data->m_pong.connected_flag)
			{
				MessageBox(L"Your license limits you to one running instance at a time.\n"
					L"To use multiple instances simultaneously, please upgrade your license.", 
					L"Already in Use!", MB_ICONASTERISK);
				return;
			}
		}
		int i = combo_box_p->GetCurSel();
		if (i > -1)
		{
			CemonComboBoxData *data = (CemonComboBoxData*)combo_box_p->GetItemData(i);
			if (data && data->m_pong.connected_flag)
			{
				MessageBox(L"This cemon is already connected to another client", 
					L"Error", MB_ICONASTERISK);
				return;
			}
			ConnectToDiscoveredDevice(data->m_addr.sin_addr.S_un.S_addr, data->m_pong);
		}
	}
}

/*
TODO:

Add function like IsTrialValid for checking
if (TCN_SELCHANGING == pnmh->code && m_enable_selchanging_check)
{
if(dlg->GetLicenseType() <= LICENSE_TYPE_TRIAL && !dlg->IsTrialValid())


*/

