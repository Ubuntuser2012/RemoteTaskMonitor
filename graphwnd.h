#ifndef GRAPHWND_H
#define GRAPHWND_H

#include <windows.h>

#include "Plot.h"

class GraphWnd : public CWnd
{
	DECLARE_DYNAMIC(GraphWnd)

	//Attributes
protected:
	HBRUSH m_hBrush;
	COLORREF m_GridColor;
	BOOL m_bGrid;
	LIST m_Graphs;
	unsigned int m_maxY;
   int m_nSize;
   int m_deltaX;
	
	
	//Methods
	void DrawGrid(HDC memDC, RECT &rect);
	BOOL Destroy();
	
public:
	//Attributes
	
	//Methods
	GraphWnd();
	~GraphWnd();
	HWND GetWindowHandle() { return m_hWnd; }
	void SetDeltaX(int deltaX);
	BOOL SetWindowPos(int x, int y, int cx, int cy);
	void ScreenToClient(HWND hwnd, RECT *rect);
	BOOL SetBkColor(COLORREF bkColor);
	BOOL SetGraphColor(long id, COLORREF LineColor);//0x00bbggrr
	COLORREF GetGraphColor(long id);
	BOOL SetGridColor(COLORREF GridColor) {m_GridColor=GridColor; return TRUE;}
	COLORREF GetGridColor(){return m_GridColor;}
	BOOL IsGridEnabled() { return m_bGrid; }
	BOOL EnableGrid(BOOL bEnable) { m_bGrid=bEnable; return TRUE;}
	BOOL AddRandomPoint(long id, unsigned int x, int y);
	BOOL AddPoint(long id, unsigned int x, int y);
	BOOL AddGraph(long id, COLORREF LineColor, int maxY);
	void RemoveGraph(long id);
	BOOL ShowWindow(int nCmdShow);
	PLOT* GetGraph(long id);
	long GetNumGraphs() const { return m_Graphs.GetCount(); }
	void Draw();
	OBJECT* GetFirst(ITERATOR &it) const;
	OBJECT* GetNext(ITERATOR &it) const;
};

#endif //GRAPHWND_H