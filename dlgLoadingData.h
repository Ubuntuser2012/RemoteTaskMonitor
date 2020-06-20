#if !defined(AFX_DLGLOADINGDATA_H__184E8D6D_39F4_4C2D_A2E4_82F9FCFFDDD4__INCLUDED_)
#define AFX_DLGLOADINGDATA_H__184E8D6D_39F4_4C2D_A2E4_82F9FCFFDDD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CDlgLoadingData.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadingData dialog

class CDlgLoadingData : public CDialog
{
// Construction
public:
	CDlgLoadingData(CWnd* pParent = NULL);   // standard constructor
   void OnCancel();
   int start(CDlgRemoteCPU* parent, DEVICE &device, STRING &file, HISTORY &history, STRING &html_fn);

// Dialog Data
	//{{AFX_DATA(CDlgLoadingData)
	enum { IDD = IDD_LOADING_DATA };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgLoadingData)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

   bool m_closed;

   int loadBegin(DEVICE &device, STRING &file, HISTORY &history, FILE **fpp, STRING &err, int &total_read);
   int loadSome(DEVICE &device, HISTORY &history, FILE *fp, STRING &err, int &total_read, CDlgRemoteCPU* parent);
   int loadEnd(DEVICE &device, HISTORY &history, FILE *fp, STRING &err, CDlgRemoteCPU* parent, STRING &outdir);

   // Generated message map functions
	//{{AFX_MSG(CDlgLoadingData)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGLOADINGDATA_H__184E8D6D_39F4_4C2D_A2E4_82F9FCFFDDD4__INCLUDED_)
