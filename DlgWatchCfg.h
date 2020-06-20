#if !defined(AFX_DLGDEBUGPROCESS_H__09CDAE35_F6DF_4F37_B1D3_4199852D0807__INCLUDED_)
#define AFX_DLGDEBUGPROCESS_H__09CDAE35_F6DF_4F37_B1D3_4199852D0807__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgDebugProcess.h : header file
//

#include "DlgDbgPs1.h"
#include "DlgDbgPs2.h"
#include "DlgDbgPs3.h"

class DEVICE;
class CDlgRemoteCPU;

#pragma warning( disable: 4996 )

/////////////////////////////////////////////////////////////////////////////
// CDlgWatchCfg dialog

class CDlgWatchCfg : public CDialog
{
	bool m_debug_new_ps;
	CDlgRemoteCPU *m_input_handler;
	DWORD m_pid;
	DWORD m_tid;
	STRING m_pname;

	static char_t m_ps_path[MAX_RFILE_PATH];
	static char_t m_map_file_path[MAX_PATH*4];
	static char_t m_email_to[MAX_PATH];

// Construction
public:
	CDlgWatchCfg(DWORD pid, wchar_t *pname, CWnd* pParent = NULL);

   static char_t* GetPsPath() { return (char_t*)m_ps_path; }
   static char_t* GetMapFilePath() { return (char_t*)m_map_file_path; }
   static char_t* GetEmail() { return (char_t*)m_email_to; }

   static void SetPsPath(const char_t* ps_path) { if(ps_path && wcslen(ps_path) < MAX_RFILE_PATH) wcscpy(m_ps_path, ps_path); }
   static void SetMapFilePath(const char_t* map_file_path) { if(map_file_path && wcslen(map_file_path) < MAX_PATH*4) wcscpy(m_map_file_path, map_file_path); }
   static void SetEmail(const char_t* email_to) { if(email_to && wcslen(email_to) < MAX_PATH) wcscpy(m_email_to, email_to); }

   static void StartCrashWatch(DEVICE* device, bool debug_new_ps, DWORD pid, const char_t *ps_path, const char_t *map_file_path, const char_t *email_to, 
                                   const char_t *ps_path2, bool watch_ps2, bool reset_device, bool silent_mode);

   DWORD GetPID() const { return m_pid; }
   bool ShouldDebugNewPs() const { return m_debug_new_ps; }

 // Dialog Data
	//{{AFX_DATA(CDlgWatchCfg)
	enum { IDD = IDD_DEBUG_PROCESS };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWatchCfg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void ShowPage(int step);
	int m_step;
	CDlgDbgPs1 m_dlgdbg1;
	CDlgDbgPs2 m_dlgdbg2;
	CDlgDbgPs3 m_dlgdbg3;

	// Generated message map functions
	//{{AFX_MSG(CDlgWatchCfg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnNext();
	afx_msg void OnPrevious();
public:
	virtual void OnCancel();
public:
   afx_msg void OnRadioDebugActiveProcess();
   afx_msg void OnRadioDebugNewProcess();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDEBUGPROCESS_H__09CDAE35_F6DF_4F37_B1D3_4199852D0807__INCLUDED_)
