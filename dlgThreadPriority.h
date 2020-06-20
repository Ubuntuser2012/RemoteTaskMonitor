#ifndef CDLGTHREADPRIORITY_H
#define CDLGTHREADPRIORITY_H

class DEVICE;

class CDlgThreadPriority : public CDialog
{
   CEdit	m_edit_prirority;
   int m_priority;
   DEVICE *m_device;
   const LIST &m_ids;

   // Construction
public:
   CDlgThreadPriority(DEVICE* device, const LIST &ids, CWnd* pParent = NULL);	// standard constructor

   // Dialog Data
   //{{AFX_DATA(CDlgThreadPriority)
   enum { IDD = IDD_THREAD_PRIORITY };

   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CDlgThreadPriority)
   virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
   virtual void OnOK();
  //}}AFX_VIRTUAL

   // Generated message map functions
   //{{AFX_MSG(CDlgThreadPriority)
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

#endif//CDLGTHREADPRIORITY_H