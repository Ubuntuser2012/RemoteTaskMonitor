#include "stdafx.h"
#include "dialog2.h"

void CDialog2::OnCancel() 
{
   m_closed=true;
   DestroyWindow();
}

