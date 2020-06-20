
#include "stdafx.h"
#include "GraphWnd2.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(GraphWnd2, CWnd)

void GraphWnd2::Size()
{
	RECT rect;
	GetClientRect(&rect);

	SetWindowPos(rect.left, rect.top, rect.right, rect.bottom);
	
}

