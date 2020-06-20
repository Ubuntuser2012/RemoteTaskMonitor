#ifndef GRAPH_H
#define GRAPH_H

#include "../ds/ds.h"
#include <windows.h>

typedef double NUMBER;

class PLOT : public OBJECT
{
	friend class GraphWnd;
   PLOT();

	int* m_x;
	int* m_y;
	int m_maxY;
	POINT*	m_control_points;//que
	int m_nPoints;//maximum size of que
	int m_nHead;//head of circular que
	int m_nTail;//tail of circular que
	long	m_id;//for user
	COLORREF m_LineColor;

   void SubHeightFromYValue(POINT* points, int numPoints, int height);
   void PutInMinMaxRange(POINT* points, int numPoints, int height);
   NUMBER ProductForPolynomialInterpolationOfX(int i, NUMBER x, int numDataPointsLessOne, const POINT* points);
   NUMBER PolynomialInterpolationOfX(NUMBER x, int numDataPointsLessOne, const POINT* points);
   void ScalePoints(int width, int height);

public:

   static PLOT* create();
	~PLOT();
	
	void DrawGraph(HDC hDC);
	void AddPoint(int x, int y);
	long InitG(long id, COLORREF LineColor, int maxY, RECT *window);
	long GetId() {return m_id;}
	COLORREF GetLineColor() { return m_LineColor; }
	void SetLineColor(COLORREF LineColor)  { m_LineColor=LineColor; }
   int GetNumPoints() const { return m_nPoints; }
   POINT* SplineFromControlPoints(POINT* controlPoints, int &numPoints);
};

#endif // GRAPH_H