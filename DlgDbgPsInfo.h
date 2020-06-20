#if !defined(AFX_DLGDBGPSINFO_H__E244748A_E781_46F5_A605_F208D45CA379__INCLUDED_)
#define AFX_DLGDBGPSINFO_H__E244748A_E781_46F5_A605_F208D45CA379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgDbgPsInfo.h : header file
//

class DEVICE;

class ADDR_INFO
{
public:
   int m_address;
   STRING m_funcname;
   int m_offset;
   STRING m_objfile;

   void set(DWORD address, STRING &funcname, DWORD offset, STRING &objfile)
   {
      m_address = address;
      m_funcname.set(funcname);
      m_offset = offset;
      m_objfile.set(objfile);
   }
};

/////////////////////////////////////////////////////////////////////////////
// CDlgDbgPsInfo dialog

class CDlgDbgPsInfo : public CDialog
{
// Construction
public:
	CDlgDbgPsInfo(DEVICE * device, CRASH_INFO *cinfo, STRING &mapfile, CWnd* pParent = NULL);   // standard constructor
   virtual ~CDlgDbgPsInfo();

   int getExceptionMsg(STRING &msg);
   
   static bool getMapFilePath(CString &path);

   // Dialog Data
	//{{AFX_DATA(CDlgDbgPsInfo)
	enum { IDD = IDD_DEBUG_PROCESS_ALERT };
	CEdit	m_edit_ps_info;
   CEdit	m_edit_map_file_path;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDbgPsInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
   STRING m_mapfile;
	DEVICE *m_device;
   CRASH_INFO* m_cinfo;

   bool copy_cinfo(CRASH_INFO *cinfo);
   int parseMapFile(STRING &mapfile, ADDR_INFO *out_addr, int num_addrs);
   ADDR_INFO *getAddrInfo1(int addr, ADDR_INFO *list, int count);
   void getExceptionCodeAsString(ULONG32 exceptionCode, STRING &ret);

   // Generated message map functions
	//{{AFX_MSG(CDlgDbgPsInfo)
   virtual void OnOK();
   virtual void OnCancel();
   virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
   afx_msg void OnParseMapFile();
   afx_msg void OnButtonMapFilePath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDBGPSINFO_H__E244748A_E781_46F5_A605_F208D45CA379__INCLUDED_)
