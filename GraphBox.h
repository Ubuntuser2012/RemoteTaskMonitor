#if !defined(AFX_GraphBox_H__B1213FAC_82C5_4C67_9544_9782FAF29DB6__INCLUDED_)
#define AFX_GraphBox_H__B1213FAC_82C5_4C67_9544_9782FAF29DB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GraphBox.h : header file
//

class GraphWnd2;

/////////////////////////////////////////////////////////////////////////////
// GraphBox dialog

class GraphBox : public CDialog
{
	DECLARE_DYNCREATE(GraphBox)

	// Construction
public:
	GraphBox(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgDbgPs1)
	enum { IDD = IDD_GRAPH_BOX };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDbgPs1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	GraphWnd2 *m_graph;

	// Generated message map functions
	//{{AFX_MSG(CDlgDbgPs1)
   virtual void OnCancel();
   virtual void OnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void Size();
	void SetTitle(const STRING& title);
	void CreateGraph(int id, int xscale, COLORREF color);
	void Draw();
	BOOL IsCreated() const;
	GraphWnd2 *GetGraphWnd();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GraphBox_H__B1213FAC_82C5_4C67_9544_9782FAF29DB6__INCLUDED_)
