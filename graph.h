#ifndef GRAPH_H
#define GRAPH_H

#include "../ds/ds.h"
#include <windows.h>

typedef double NUMBER;

class CGraph : public OBJECT
{
	friend class CGraphWnd;
   CGraph();

	int* m_x;
	int* m_y;
	int m_maxY;
	POINT*	m_pPoints;//que
	int m_nPoints;//maximum size of que
	int m_nSize;//current number of points in que
	int m_nHead;//head of circular que
	int m_nTail;//tail of circular que
	long	m_id;//for user
	COLORREF m_LineColor;

   void ScalePoints(int &x0, int i, int &j, int last, int width, int height);
   void SubHeightFromYValue(POINT* points, int numPoints, int height);
   void PutInMinMaxRange(POINT* points, int numPoints, int height);
   NUMBER ProductForPolynomialInterpolationOfX(int i, NUMBER x, int numDataPointsLessOne, const POINT* points);
   NUMBER PolynomialInterpolationOfX(NUMBER x, int numDataPointsLessOne, const POINT* points);

public:

   static CGraph* create();
	~CGraph();
	
	void DrawGraph(HDC hDC);
	void AddPoint(int x, int y);
	void AddPoints(int *x, int *y, int num_points);
	long InitG(long id, COLORREF LineColor, int maxY, RECT *window);
	long GetId() {return m_id;}
	COLORREF GetLineColor() { return m_LineColor; }
	void SetLineColor(COLORREF LineColor)  { m_LineColor=LineColor; }
   int GetSize() const { return m_nSize; }
   int GetNumPoints() const { return m_nPoints; }
   POINT* SplineFromControlPoints(POINT* controlPoints, int &numPoints);
};

#endif // GRAPH_H