// DlgSelectCore.cpp : implementation file
//

#include "stdafx.h"
#include "../ds/ds.h"
extern "C" {
#include "comm.h"
}
#include "thread.h"
#include "threadlist.h"
#include "process.h"
#include "processlist.h"
#include "watchlist.h"
#include "device.h"
#include "devicelist.h"
#include "remotecpu.h"
#include "listctrl2.h"
#include "dlgRemotecpu.h"
#include "dlgThreadPriority.h"
#include "dlgStartProcess.h"
#include "dlgWatchCfg.h"
#include "dlgWatchBase.h"
#include "dlgWatchCPU.h"
#include "watchlist.h"

#include "DlgSelectCore.h"

IMPLEMENT_DYNAMIC(DlgSelectCore, CDialog)

DlgSelectCore::DlgSelectCore(const LIST &ids, int num_cores, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SELECT_CORE, pParent)
	, m_ids(ids)
	, m_num_cores(num_cores)
{

}

DlgSelectCore::~DlgSelectCore()
{
}

void DlgSelectCore::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DlgSelectCore, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// DlgSelectCore message handlers
BOOL DlgSelectCore::OnInitDialog()
{
	BOOL ret = CDialog::OnInitDialog();
	if (!ret)
	{
		return false;
	}
	CComboBox * combo_box_p = (CComboBox*)GetDlgItem(IDC_COMBO_CORE);
	for (int i = 0; i < m_num_cores; ++i)
	{
		STRING s;
		s.sprintf(L"%i", i + 1);
		combo_box_p->AddString(s.s());
	}
			

	return ret;
}

void DlgSelectCore::OnOK()
{
	CComboBox * combo_box_p = (CComboBox*) GetDlgItem(IDC_COMBO_CORE);
	int sel = combo_box_p->GetCurSel();
	if (sel != CB_ERR)
	{
		int core = sel + 1;

		ITERATOR it;
		int i;
		EXECOBJ *execobj = 0;
		for (i = 0, execobj = (EXECOBJ *)m_ids.GetFirst(it); i < m_ids.GetCount(); execobj = (EXECOBJ *)m_ids.GetNext(it), i++)
		{
			if (execobj)
			{
				PROCESS *p = 0;
				THREAD *t = 0;
				if (execobj->getType() == EXECOBJ_PROCESS)
					p = dynamic_cast<PROCESS*>(execobj);
				else if (execobj->getType() == EXECOBJ_THREAD)
					t = dynamic_cast<THREAD*>(execobj);

				if (p)
				{
					p->setAffinity(core);
				}
				else if (t)
				{
					t->setAffinity(core);
				}
			}
		}
	}
	CDialog::OnOK();
}
void DlgSelectCore::OnCancel()
{
	CDialog::OnCancel();
}
void DlgSelectCore::OnDestroy()
{
	CDialog::OnDestroy();
}
