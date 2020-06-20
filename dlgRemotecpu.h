// RemoteCPUDlg.h : header file
//

#if !defined(AFX_RemoteCPUDLG_H__6BBFD0F2_7436_404F_8FCD_3F5F25F7BFE8__INCLUDED_)
#define AFX_RemoteCPUDLG_H__6BBFD0F2_7436_404F_8FCD_3F5F25F7BFE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "tabctrl2.h"
#include "devicelist.h"
#include "editurl.h"
#include "license.h"
#include "gdgraph.h"
#include "history.h"
#include "listctrl2.h"
#include "GraphWnd2.h"
#include "PropPageAbout.h"
#include "PropPageDevice.h"
#include "PropPageCpuGraphs.h"
#include "PropPageMemGraphs.h"
#include "CpuGraphs.h"
#include "PropPageOptions.h"
#include "PropPageProcesses.h"
#include "PropPagePython.h"
#include "PropSheet.h"
#include "DlgError.h"
#include "DeviceDiscoverer.h"

class DEVICE;
class DISPLAY_LIST;

#define MAX 200
#define IPLEN 64
#define PORTMAX 6

enum CONNECTION_DATA_STATE
{
	CONNECTION_DATA_STATE__NONE,
	CONNECTION_DATA_STATE__CONNECTING,
	CONNECTION_DATA_STATE__CONNECTED,
	CONNECTION_DATA_STATE__FAILED,
	CONNECTION_DATA_STATE__DISCONNECTING
};

enum {
	MONITOR_DEVICE_TIMER_ID=1,
	GET_SNAPSHOT_TIMER_ID,
	SET_GET_MEMORY_INFO_TIMER_ID,
	TAB_CHANGED_TIMER,
	PY_AT_STARTUP,
	CANCEL_PS_WATCHES,
	CANCEL_MEM_WATCH,
	CANCEL_CPU_WATCH,
	UPDATE_TRIAL_TIME_REMAINING_TIMER_ID,
	DEVICE_DISCOVERY_TIMER_ID,
	CLIENT_DISCOVERY_TIMER_ID,
};

typedef struct CONNECTION_DATA_
{
	unsigned long ip;
	unsigned short port;
	SOCKET so;
	unsigned char state;
	CDlgRemoteCPU* parent;
	HANDLE thread;
	DWORD start_time;
}
CONNECTION_DATA;

enum TABPAGE
{
	TABPAGE_DEVICE,
	TABPAGE_PROCESSES,
	TABPAGE_CPU_GRAPHS,
	TABPAGE_MEM_GRAPHS,
	//TABPAGE_POWER_LEVEL_GRAPHS,
	//TABPAGE_CORES_GRAPHS,
	TABPAGE_OPTIONS,
	TABPAGE_PYTHON,
	TABPAGE_ABOUT,
	TABPAGES_TOTAL,
};

struct EMAIL_DATA
{
	CDlgRemoteCPU *m_mainwnd;
	short m_port;
	STRING m_server;
	STRING m_from;
	STRING m_to;
	STRING m_subject;
	STRING m_body;

	EMAIL_DATA::EMAIL_DATA(CDlgRemoteCPU *wnd, short port, const STRING &server, const STRING &from, const STRING &to, const STRING &subject, const STRING &body)
		: m_mainwnd(wnd)
		, m_port(port)
		, m_server(server)
		, m_from(from)
		, m_to(to)
		, m_subject(subject)
		, m_body(body)
	{
	}

};

enum
{
	POWER_PLUG_DISCONNECTED,
	POWER_PLUG_CONNECTED,
	POWER_PLUG_TALKING1,
	POWER_PLUG_TALKING2,
	POWER_PLUG_TALKING3,
	POWER_PLUG_TALKING4,
	NUM_PLUG_ICONS
};
/////////////////////////////////////////////////////////////////////////////
// CDlgRemoteCPU dialog

#define NUM_WINDOWS_TO_RESIZE 1

class CDlgRemoteCPU : public CDialog
{
	// Construction
public:
	CDlgRemoteCPU(CWnd* pParent = NULL);	// standard constructor
	virtual ~CDlgRemoteCPU();
	OBJECT* GetDisplayListObject(int i);
	int GetNumProcessors();
	void SetLicenseKeyUI(const STRING &key);
	void SetLicenseKeyUI2();
	void Connect(unsigned long ip, unsigned short port);
	void Disconnect();
	void SortBy(SORT_BY sortTechnique);
	DEVICE* GetDevice(int i) const;
	float GetTotalCpu() const;
	int GetRedCpu() const { return m_red_cpu; }
	int GetSnapInterval();
	const LICENSE &GetLicense() const { return m_license; }
	int GetLicenseType() const { return m_license.GetLicenseType(); }
	bool IsLicenseValid() const { return m_license.IsLicenseValid(); }
	bool TrialExpired() const { return m_license.TrialExpired(); }
    int TrialSecondsElapsedSinceStart() const { return m_license.TrialSecondsElapsedSinceStart(); }
	CWnd* GetFieldWnd(int fieldID);
	int GetLogDuration() { return m_log_duration; }
	int ProcessWinMessages(CWnd* wnd);
	bool GetPromptShowReport() const { return m_prompt_show_report; }
	void SetPromptShowReport(bool prompt_show_report) { m_prompt_show_report = prompt_show_report; }
	bool GetProcessingScript() const { return m_processing_script; }
	void SetProcessingScript(bool processing_script);
	bool SetSnapInterval(const char_t *snap_interval, STRING &err);
	void OnChangedTab(int activeTab);
	void OnButtonSaveSnapshot();
	void OnButtonCaptureStart();
	void OnButtonCaptureStop();
	void OnButtonSetCemonPriority();
	void OnCheckShowThreads();
	void OnCheckShowMemory(bool state); 
	void HandleConnection(ULONG ip, unsigned short port);
	void OnButtonResetDevice();
	STRING OnGenerateReport(bool restart_history);
	void OnButtonResetServer();
	void OnButtonLoadCapturedData();
	bool OnCheckCaptureOnDev(BOOL capture);
	bool OnCheckDontPromptShowReport(BOOL prompt_show_report);
	void OnButtonKey(char_t *typed_key);
	static DWORD WINAPI ShellExecuteThread(LPVOID lpParameter);
	static DWORD WINAPI SendEmailThread(LPVOID lpParameter);
    void SendEmail(const STRING &to, const STRING &subject, const STRING &body);
	bool GetLocalAppDataDirectory(STRING &dir) const;
	bool GetMyCurrentDirectory(STRING &sdir) const;
    void LicenceTest();
	void AddCpuGraph(PLOT_DATA * plot_item);
	void AddMemGraph(PLOT_DATA * plot_item);
	int GetNumDiscoveredDevices() const;
	void OnConnectButtonClicked();

	// Dialog Data
	CDlgError m_dlgerr;
	CPropPageDevice m_device;
	CPropPageProcesses m_processes;
	PropPageCpuGraphs m_cpu_graphs;
	PropPageMemGraphs m_mem_graphs;
	CPropPageOptions m_options;
    CPropPagePython m_python;
	CPropPageAbout m_about;
	//{{AFX_DATA(CDlgRemoteCPU)
	enum { IDD = IDD_REMOTECPU_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgRemoteCPU)

protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
	void AdjustChildWindows();
	void RemmemberWindowSize();
	void ShowSysInfo(float cpu, float memLoad, const MY_SYSTEM_INFO *sysinfo);
	BOOL CfgLoad();
	BOOL CfgSave();
	void CloseDevices();
	void Connecting(int state, unsigned long ip, unsigned short port);
	void EnableDevicePageButtons(bool enable);
	void UpdateDevicePageButtons(DEVICE *device);
	void ReadDevices();
	int ReadDevice(DEVICE *device, STRING &err);
	void UpdateDisplayList(DEVICE *device);
	void ConnectionStateMachine();
	void SetButtonsState(int state);
	void GetSnapShot();
	void SaveSnapShot();
	void SetGetMemoryInfo();
	bool ValidateSnapInterval(int snap_interval, STRING &err);
	void StartCapture(int snap_interval, bool prompt_on_err);
	void SetCemonPriority(DEVICE *device, int priority);
	int LicenseCheck();
	void ConnectToDiscoveredDevice(ULONG ip, DISCOVERY_PONG &pong);
	void UpdateDiscoveredServers(sockaddr_in addr, DISCOVERY_PONG &pong);
	void RemoveDeadServers();
	void SetCaptureOnDev(bool enabled);
	void SetEditUrl(const char_t *val);
	void GetSnapInterval(char_t *data, int len);
	void GetEditUrl(char_t *data, int len);
	void UpdateFields(BOOL bSaveAndValidate);
	void EnableField(int fieldID, BOOL bEnable);
	void SetFieldText(int fieldID, char_t *val);
	void SetButtonImage(int fieldID, int img_id);
    void PathFromTokens(STRING &path, STRING *token, int num_tokens);
	CListCtrl2 &GetDisplayList() { return m_List; }
	void SizeTabSheet();
	void ToggleFeatures();
	void LoadPlugIcons();
	void UnLoadPlugIcons();
	static DWORD ConnectThread( LPVOID lpThreadParameter);
    static bool GetHostByName(STRING &domain_name, STRING &ip, int &error);
    bool OkToPrompt();
    void SetTrialUIMessage(STRING &msg, int msRemaining, bool connected);
    void KillTimers();
	void CreateGraphs(int num_processors);
	void UpdateGraphs(unsigned int x, const DEVICE *device, int num_processors);
	STRING GetRtmVersion() const;
	bool IsConnected() const;
	void EndReport(const PLOT_DATA *plot_item);
	void DiscoveryInit();

	// Generated message map functions
	//{{AFX_MSG(CDlgRemoteCPU)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR  nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	HICON m_hIcon;
	//CTabCtrl2 m_Tabs;
	CONNECTION_DATA m_con;
	DEVICE_LIST m_devices;
	int m_dl_index;
	DISPLAY_LIST m_display_list[2];
	DISPLAY_LIST *mp_display_list;
	int m_gap[NUM_WINDOWS_TO_RESIZE][2];
	int m_window_resize[NUM_WINDOWS_TO_RESIZE][2];
	int m_window_id[NUM_WINDOWS_TO_RESIZE];
	unsigned int m_t;//timer ticker for graph's x-axis
	unsigned int m_xscale;
	int m_red_cpu;
	bool m_py_at_startup;
	bool m_prompt_show_report;
	int m_checkbox_memory;
	int m_log_duration;
	bool m_set_cemon_priority;
	bool m_save_snapshot;
	bool m_get_thread_names;
	LICENSE m_license;
	HISTORY m_history;
	CListCtrl2 m_List;
	CPropSheet m_tabs;
	DWORD m_connected_at;
	bool m_processing_script;
	DeviceDiscoverer m_device_discoverer;
	LIST m_discovered_devices;
	HBITMAP m_plug[NUM_PLUG_ICONS];
	bool m_destroying;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RemoteCPUDLG_H__6BBFD0F2_7436_404F_8FCD_3F5F25F7BFE8__INCLUDED_)
