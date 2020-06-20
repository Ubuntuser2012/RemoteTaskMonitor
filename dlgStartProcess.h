#ifndef CDLGSTARTPROCESS_H
#define CDLGSTARTPROCESS_H

#pragma warning( disable: 4996 )

class CDlgStartProcess : public CDialog
{
   CEdit	m_edit_path;
	static char_t m_ps_path[MAX_RFILE_PATH];

   // Construction
public:
   static char_t* GetPsPath() { return (char_t*)m_ps_path; }
   static void SetPsPath(const char_t* ps_path) { if(ps_path && wcslen(ps_path) < MAX_RFILE_PATH) wcscpy(m_ps_path, ps_path); }
   CDlgStartProcess(CWnd* pParent = NULL);	// standard constructor

   // Dialog Data
   //{{AFX_DATA(CDlgStartProcess)
   enum { IDD = IDD_START_PROCESS };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgStartProcess)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

   // Generated message map functions
   //{{AFX_MSG(CDlgStartProcess)
   virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

#endif//CDLGSTARTPROCESS_H
