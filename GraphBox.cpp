// GraphBox.cpp : implementation file
//

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../ds/ds.h"
extern "C"{
#include "comm.h"
}
#include "../common/util.h"
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "Plot.h"
#include "GraphWnd2.h"


#include "RemoteCPU.h"
#include "GraphBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GraphBox dialog

IMPLEMENT_DYNCREATE(GraphBox, CDialog)


GraphBox::GraphBox(CWnd* pParent /*=NULL*/)
	: CDialog(GraphBox::IDD, pParent)
	, m_graph(0)
{
	//{{AFX_DATA_INIT(GraphBox)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void GraphBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDbgPs1)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(GraphBox, CDialog)
	//{{AFX_MSG_MAP(CDlgDbgPs1)
	//}}AFX_MSG_MAP
	ON_WM_CREATE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// GraphBox message handlers
void GraphBox::OnOk()
{
}

void GraphBox::OnCancel()
{
}


BOOL GraphBox::OnInitDialog()
{
	CDialog::OnInitDialog();

	Size();
	GetDlgItem(IDC_STATIC_GRAPH_BOX)->SetWindowTextW(TXT("Core"));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void GraphBox::Size()
{
	if (m_graph)
	{
		RECT rect;
		GetClientRect(&rect);
		GetDlgItem(IDC_STATIC_GRAPH_BOX)->SetWindowPos(0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);

		rect.top += 15;
		rect.left += 7;
		m_graph->SetWindowPos(rect.left, rect.top, 
			rect.right - rect.left - 7, 
			rect.bottom - rect.top - 5);
	}
//	RedrawWindow();//todo is this necessary?
}

void GraphBox::SetTitle(const STRING& title)
{
	GetDlgItem(IDC_STATIC_GRAPH_BOX)->SetWindowText(title.s());
}

void GraphBox::CreateGraph(int id, int xscale, COLORREF color)
{
	CString strMyClass = AfxRegisterWndClass(CS_VREDRAW |
		CS_HREDRAW,
		::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH) ::GetStockObject(WHITE_BRUSH),
		::LoadIcon(NULL, IDI_APPLICATION));

	m_graph = new GraphWnd2;

	RECT rect;
	GetDlgItem(IDC_STATIC_GRAPH_BOX)->GetClientRect(&rect);
	m_graph->Create(strMyClass, NULL, WS_CHILD | WS_VISIBLE, rect, GetDlgItem(IDC_STATIC_GRAPH_BOX), 0, NULL);
	m_graph->SetDeltaX(xscale);
	m_graph->AddGraph(id, color, 100);
}

void GraphBox::Draw()
{
	m_graph->Draw();
}

BOOL GraphBox::IsCreated() const
{
	return m_graph ? true : false;
}

GraphWnd2 *GraphBox::GetGraphWnd()
{
	return m_graph;
}

