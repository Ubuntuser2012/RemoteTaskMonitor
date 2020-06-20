#include "stdafx.h"
#include "GraphWnd.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "../common/util.h"

#pragma warning( disable: 4996 )

#define DEFAULT_GRID_COLOR 0x00003e12
#define BACKGROUND_COLOR 0x000a0a0a

IMPLEMENT_DYNAMIC(GraphWnd, CWnd)

int g_oldy=0;

GraphWnd::GraphWnd()
{
	m_hBrush = NULL;
	m_bGrid = TRUE;
	m_GridColor = DEFAULT_GRID_COLOR;
   m_deltaX = 1;
   m_nSize =0;
}

GraphWnd::~GraphWnd()
{
	Destroy();
}

BOOL GraphWnd::Destroy()
{
	DestroyWindow();
    m_Graphs.Destroy(TRUE);

	return TRUE;
}

BOOL GraphWnd::SetBkColor(COLORREF bkColor)
{
	BOOL bRet = FALSE;

	if(m_hWnd)
	{
		DeleteObject(m_hBrush);
		m_hBrush = CreateSolidBrush(bkColor);
		SendMessage(WM_ERASEBKGND, 0, 0);
		bRet = TRUE;
	}
   	return bRet;
}


void GraphWnd::ScreenToClient(HWND hwnd, RECT *rect)
{
   POINT p1, p2;
   p1.x = rect->left;
   p1.y = rect->top;
   p2.x = rect->right;
   p2.y = rect->bottom;
   ::ScreenToClient(hwnd, &p1);
   ::ScreenToClient(hwnd, &p2);
   rect->left = p1.x;
   rect->top = p1.y;
   rect->right = p2.x;
   rect->bottom = p2.y;
}

BOOL GraphWnd::SetWindowPos(int x, int y, int cx, int cy)
{
	if(::SetWindowPos(m_hWnd, 0, x, y, cx, cy, SWP_SHOWWINDOW))
	{
		UpdateWindow();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL GraphWnd::AddGraph(long id, COLORREF LineColor, int maxY)
{
	PLOT* g = GetGraph(id);
	if(g)
	{
		//g->m_nSize = GetPoints(id, g->m_x, g->m_nPoints, g->m_y);
	}
	else
	{
      int i;
      int numPoints = 0;
      PLOT * g = PLOT::create();
		if(!g)
		{
			return FALSE;
		}
       m_Graphs.Insert(g);
	   RECT rect;
	   GetClientRect(&rect);
	   g->InitG(id, LineColor, maxY, &rect);
       numPoints = g->GetNumPoints();
		m_maxY=rect.bottom - rect.top;

		return TRUE;
	}
	return FALSE;
}

void GraphWnd::RemoveGraph(long id)
{
   PLOT * g = GetGraph(id);
   if(g)
   {
      m_Graphs.Remove(g);
      g->deref();
   }
}

PLOT* GraphWnd::GetGraph(long id)
{
   ITERATOR it;
   PLOT * g;
   for(g=(PLOT *)m_Graphs.GetFirst(it); g; g=(PLOT *)m_Graphs.GetNext(it))
   {
      if(id==g->GetId())
         return g;
   }
   return NULL;
}

OBJECT* GraphWnd::GetFirst(ITERATOR &it) const
{
	return m_Graphs.GetFirst(it);
}

OBJECT* GraphWnd::GetNext(ITERATOR &it) const
{
	return m_Graphs.GetNext(it);
}

BOOL GraphWnd::SetGraphColor(long id, COLORREF LineColor)
{
	PLOT* pGraph = GetGraph(id);

	if(pGraph) 
	{
		pGraph->SetLineColor(LineColor);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

COLORREF GraphWnd::GetGraphColor(long id)
{
	PLOT* pGraph = GetGraph(id);

	if(pGraph) 
	{
		return pGraph->GetLineColor();
	}
	else
	{
		return 0;
	}
}

BOOL GraphWnd::ShowWindow(int nCmdShow)
{
	return ::ShowWindow(m_hWnd, nCmdShow);
}

void GraphWnd::SetDeltaX(int deltaX)
{
	m_deltaX = deltaX;
	m_hBrush = CreateSolidBrush(BACKGROUND_COLOR);
}


void GraphWnd::Draw()
{
	HDC memDC;
	HBITMAP hbmp;
	RECT rect;
	CDC* cdc = GetDC();
	GetClientRect(&rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	if(height>0 && width>0 && cdc)
	{
		memDC	= CreateCompatibleDC(cdc->m_hDC);
		hbmp	= CreateCompatibleBitmap(cdc->m_hDC, width, height);
		
		SelectObject(memDC, hbmp);
		SelectObject(memDC, m_hBrush);
		
		PatBlt(memDC, rect.left, rect.top, width, height, PATCOPY);
		
		if(m_bGrid) DrawGrid(memDC, rect);
		
		g_oldy = height/2;
		
		ITERATOR it;
		PLOT * g;
		for(g=(PLOT *)m_Graphs.GetFirst(it); g; g=(PLOT *)m_Graphs.GetNext(it))
		{
			g->DrawGraph(memDC);
		}
		
		BitBlt(cdc->m_hDC, rect.left, rect.top, width, height, memDC, rect.left, rect.top, SRCCOPY);

		DeleteDC(memDC);
		DeleteObject(hbmp);
	}
	if (cdc)
	{
		ReleaseDC(cdc);
	}
}

void GraphWnd::DrawGrid(HDC memDC, RECT &rect)
{
	HPEN hpen = CreatePen(PS_SOLID, 1, m_GridColor);
	SelectObject(memDC, hpen);

	int width = rect.right-rect.left;
	int height = rect.bottom-rect.top;

	for(int i=rect.bottom; i>=0; i-= (height *5/100))
	{
		MoveToEx(memDC, rect.left, i, NULL);
		LineTo(memDC, rect.right, i);
	}
	DeleteObject(hpen);
}

BOOL GraphWnd::AddRandomPoint(long id, unsigned int x, int y)
{
	RECT rect;
	int rnd;
	int width;
	int height;

	PLOT* pGraph = GetGraph(id);
	if(!pGraph)
	{
		return FALSE;
	}
	else
	{
      GetClientRect(&rect);
		
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
		
		if(!height)
			return FALSE;
		
		rnd = rand() % m_maxY;
		g_oldy = rnd;
		rnd = rnd - m_maxY/2;
		
		pGraph->AddPoint(x*m_deltaX, rnd);
		
		return TRUE;
	}
}

BOOL GraphWnd::AddPoint(long id, unsigned int x, int y)
{
	RECT rect;
	int width;
	int height;

	PLOT* pGraph = GetGraph(id);
	if(!pGraph)
	{
		return FALSE;
	}
	else
	{
      GetClientRect(&rect);
		
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
		
		if(!height)
			return FALSE;
		
		pGraph->AddPoint(x*m_deltaX, y);

      if(m_nSize>=0 && pGraph->GetNumPoints() > m_nSize)
      {
         m_nSize = pGraph->GetNumPoints();
      }
		
		return TRUE;
	}
}
