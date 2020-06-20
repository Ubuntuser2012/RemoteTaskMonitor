// listviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "listview.h"
#include "listviewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ClistviewDlg dialog




ClistviewDlg::ClistviewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ClistviewDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ClistviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ClistviewDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ClistviewDlg message handlers

BOOL ClistviewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
   m_List.Create(
      WS_CHILD | 
      WS_VISIBLE | 
      WS_BORDER | 
      LVS_REPORT | 
      LVS_OWNERDATA, 
      CRect(10, 10, 320, 280), this, 0x285);
   m_List.SetExtendedStyle(
      LVS_EX_FULLROWSELECT | 
      LVS_EX_GRIDLINES |
      LVS_EX_HEADERDRAGDROP|
      LVS_EX_FLATSB|
      LVS_EX_INFOTIP);

   m_ImageList.Create(16, 16, ILC_MASK, 1, 1);
   m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_ICON1));

   m_List.SetImageList(&m_ImageList, LVSIL_SMALL);

   LVCOLUMN lvColumn;
   int nCol = 0;

   lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_IMAGE;
   lvColumn.fmt = LVCFMT_LEFT;
   lvColumn.cx = 120;
   lvColumn.iImage = 0;
   lvColumn.pszText = L"1st Name";
   nCol = m_List.InsertColumn(nCol, &lvColumn);

   lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_IMAGE;
   lvColumn.fmt = LVCFMT_LEFT;
   lvColumn.cx = 120;
   lvColumn.iImage = 0;
   lvColumn.pszText = L"2nd Name";
   nCol = m_List.InsertColumn(++nCol, &lvColumn);
/*
   LVITEM lvItem;
   lvItem.mask = LVIF_TEXT;
   lvItem.iItem = 0;
   lvItem.iSubItem = 0;
   lvItem.pszText = L"Sandra";
   m_List.InsertItem(&lvItem);
   m_List.SetItemText(lvItem.iItem, 1, L"Anschwitz");


   lvItem.mask = LVIF_TEXT;
   lvItem.iItem = 1;
   lvItem.iSubItem = 0;
   lvItem.pszText = L"James";
   m_List.InsertItem(&lvItem);
   m_List.SetItemText(lvItem.iItem, 1, L"Bond");
*/
   m_List.ShowWindow(SW_SHOW);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL ClistviewDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
   int i = LVN_SETDISPINFO;
   NMLVDISPINFO *dispInfo = (NMLVDISPINFO *)lParam;
   if(LVM_GETITEM == dispInfo->hdr.code)
   {
      LV_ITEM* pItem= &(dispInfo)->item;
      if (pItem->mask & LVIF_TEXT) //valid text buffer?
      {
         switch(pItem->iSubItem)
         {
         case 0: //fill in main text
            lstrcpy(pItem->pszText, L"Adam");
            break;
         case 1: //fill in sub item 1 text
            lstrcpy(pItem->pszText, L"Morgan");
            break;
         case 2: //fill in sub item 2 text
            break;
         }
         return 1;
      }
      else if (pItem->mask & LVIF_IMAGE) //valid image?
      {
         //pItem->iImage = m_Items[iItemIndx].m_iImageIndex;
      }
   }

   return CDialog::OnNotify(wParam, lParam, pResult);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void ClistviewDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR ClistviewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

