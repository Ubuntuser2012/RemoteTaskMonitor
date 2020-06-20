// listviewDlg.h : header file
//

#pragma once


// ClistviewDlg dialog
class ClistviewDlg : public CDialog
{
// Construction
public:
	ClistviewDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LISTVIEW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
   CListCtrl m_List;
   CImageList m_ImageList;

	// Generated message map functions
	virtual BOOL OnInitDialog();
   virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};

/*
BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
END_MESSAGE_MAP()

In the handler for the LVN_GETDISPINFO notification message, you must check to see what type of information is being requested. The possible values are:

*

LVIF_TEXT   The pszText member must be filled in.
*

LVIF_IMAGE   The iImage member must be filled in.
*

LVIF_INDENT   The iIndent member must be filled in.
*

LVIF_PARAM   The lParam member must be filled in.
*

LVIF_STATE   The state member must be filled in.

You should then supply whatever information is requested back to the framework.

The following example (taken from the body of the notification handler for the list control object) demonstrates one possible method by supplying information for the text buffers and image of an item:
Copy

LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
LV_ITEM* pItem= &(pDispInfo)->item;

int iItemIndx= pItem->iItem;

if (pItem->mask & LVIF_TEXT) //valid text buffer?
{
switch(pItem->iSubItem){
case 0: //fill in main text
lstrcpy(pItem->pszText, 
m_Items[iItemIndx].m_strItemText);
break;
case 1: //fill in sub item 1 text
lstrcpy(pItem->pszText,
m_Items[iItemIndx].m_strSubItem1Text);
break;
case 2: //fill in sub item 2 text
lstrcpy(pItem->pszText,
m_Items[iItemIndx].m_strSubItem2Text);
break;
}
}

if pItem->mask & LVIF_IMAGE) //valid image?
pItem->iImage= 
m_Items[iItemIndx].m_iImageIndex;
*/