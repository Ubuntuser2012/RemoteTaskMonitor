#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include "graph.h"
extern "C" {
#include "../cubicspline/cubicspline.h"
}
#define GRAPHLINE_COLOR 0x0000FF00

// Polynomial interpolation - Spline
// http://en.wikipedia.org/wiki/Polynomial_interpolation
// p(x) = Sum[i=0 to n]( y(i) . Product[j=0 to n,j!=i]( (x - x(j)) / (x(i) - x(j) ) ) )


NUMBER CGraph::ProductForPolynomialInterpolationOfX(int i, NUMBER x, int numControlPointsLessOne, const POINT* controlPoints)
{
   int j;
   NUMBER ret = 1;
   for(j=0; j<=numControlPointsLessOne; j++)
   {
      if(j!=i)
      {
         NUMBER r = (x - controlPoints[j].x) / (controlPoints[i].x - controlPoints[j].x);
         ret *= r;
      }
   }
   return ret;
}

NUMBER CGraph::PolynomialInterpolationOfX(NUMBER x, int numControlPointsLessOne, const POINT* controlPoints)
{
   int i;
   NUMBER ret = 0;
   for(i=0; i<=numControlPointsLessOne; i++)
   {
      ret += (controlPoints[i].y * ProductForPolynomialInterpolationOfX(i, x, numControlPointsLessOne, controlPoints));
   }
   return ret;
}

NUMBER Sx(POINT *p, int n, NUMBER x)
{
   int i;
   NUMBER *h = (NUMBER*) malloc(n * sizeof(NUMBER));
   NUMBER *b = (NUMBER*) malloc(n * sizeof(NUMBER));
   NUMBER *u = (NUMBER*) malloc(n * sizeof(NUMBER));
   NUMBER *v = (NUMBER*) malloc(n * sizeof(NUMBER));
   NUMBER *z = (NUMBER*) malloc(n * sizeof(NUMBER));
   NUMBER Ci, Di, Sx;
//Compute the hi and bi
   for(i = 0; i< n; i++)
   {
      h[i] = p[i+1].x - p[i].x;
      b[i] = (p[i+1].y - p[i].y)/h[i];
   }
//Gaussian Elimination
   u[0] = v[0] = 0;
   u[1] = 2 *(h[0] + h[1]);
   v[1] = 6 * (b[1] - b[0]);
   for(i = 2; i<n; i++)
   {
      u[i] = 2 * (h[i- 1] + h[i]) - ( (h[i-1] * h[i-1]) / u[i-1] );
      v[i] = 6 * (b[i] - b[i-1]) - ( h[i-1] * v[i-1] / u[i-1] );
   }
//Back-substitution
//zn = 0 ?
   for(i = n-1; n>0; n--)
   {
      z[i] = (v[i] - h[i]*z[i+1])/u[i];
   }
   z[0] = 0;

   Ci = p[i+1].y/h[i] - h[i]*z[i+1]/6;
   Di = p[i].y/h[i] - h[i]*z[i]/6;
   Sx = z[i]*(p[i+1].x - x)*(p[i+1].x - x)*(p[i+1].x - x)/(6*h[i]) +
      z[i+1]*(x - p[i].x)*(x - p[i].x)*(x - p[i].x)/(6*h[i]) +
      Ci*(x - p[i].x) +
      Di*(p[i+1].x - x);
   free(h);
   free(b);
   free(u);
   free(v);
   free(z);
   return Sx;
}

//////////////////////////////////////////////////////////////////////

CGraph* CGraph::create() 
{
   CGraph* o = new CGraph;
   if(!o)
      throw 0;
   o->m_ref++;
   return o;
}

CGraph::CGraph()
{
	m_LineColor = GRAPHLINE_COLOR;
	m_pPoints = NULL;
	m_nPoints = 0;
	m_nHead = 0;
	m_nTail = 0;
	m_nSize = 0;
	m_x=0;
	m_y=0;
}

CGraph::~CGraph()
{
	if(m_pPoints)
		free(m_pPoints);
	if(m_x)
		free(m_x);
	if(m_y)
		free(m_y);
}

long CGraph::InitG(long id, COLORREF LineColor, int maxY, RECT *window)//long nPoints, unsigned int maxY)
{
	m_nHead = 0;
	m_nTail = 0;
	m_nSize = 0;
	m_id=id;
	m_LineColor = LineColor;
	m_maxY = maxY;
	m_nPoints = window->right - window->left;
	m_pPoints = (POINT*) malloc(m_nPoints * sizeof(POINT));
	memset(m_pPoints, 0, m_nPoints * sizeof(POINT));
	m_x = (int *) malloc(m_nPoints * sizeof(int));
	int i;
	for(i = 0; i < m_nPoints; i++)
		m_x[i] = i;
	m_y = (int*) malloc(m_nPoints * sizeof(int));
	memset(m_y, 0, m_nPoints * sizeof(int));
	return (m_nPoints && m_x && m_y) ? 1: 0;
}

void CGraph::AddPoint(int x, int y)
{
	if(m_nPoints)//que max size is not zero
	{
		m_x[m_nHead] = x;
		m_y[m_nHead] = y;
		
		m_nHead = (m_nHead + 1) % m_nPoints;
		
		if(m_nSize == m_nPoints)//que is full
		{
			m_nTail = (m_nTail + 1) % m_nPoints;
		}
		else
		{
			m_nSize++;
		}
	}
}

void CGraph::AddPoints(int *x, int *y, int num_points)
{
	do
	{
		if(m_nPoints && (m_nSize+num_points) <= m_nPoints)//que max size is not zero
		{
			memcpy(&m_x[m_nSize], x, num_points * sizeof(unsigned int ));
			memcpy(&m_y[m_nSize], y, num_points * sizeof(int));

			m_nSize += num_points;
			break;
		}
		else {
			m_nPoints = m_nSize+num_points;
			m_pPoints = (POINT*) realloc(m_pPoints, m_nPoints * sizeof(POINT));
			m_x = (int *) realloc(m_x, m_nPoints * sizeof(int));
			m_y = (int*) realloc(m_y, m_nPoints * sizeof(int));
		}
	}
	while(1);
}

void CGraph::SubHeightFromYValue(POINT* points, int numPoints, int height)
{
   for(int i=0; i<numPoints; i++)
   {
      int y = height - points[i].y;
      points[i].y = y;
   }
}

void CGraph::PutInMinMaxRange(POINT* points, int numPoints, int height)
{
   for(int i=0; i<numPoints; i++)
   {
      int y = points[i].y;
      if(y < 0)
         y = 0;
      else if(y > height)
         y = height;
      points[i].y = y;
   }
}

void CGraph::ScalePoints(int &x0, int i, int &numPoints, int last, int width, int height)
{
   if((x0 == -1) && (m_x[last] - m_x[i] < width))
      x0 = i;

   if(x0>-1)
   {
      m_pPoints[numPoints].x = m_x[i] - m_x[x0];

      int y = m_y[i];
      y = y * height / (int)m_maxY;
      //y = height-y;
      m_pPoints[numPoints].y = y;
      numPoints++;
   }
   else
   {
      int xx=0;
      xx=1;
   }
}

POINT* CGraph::SplineFromControlPoints(POINT* controlPoints, int &numPoints)
{
	POINT *splinePoints;
	int numSplinePoints;
	double *out;
	double *in = new double [numPoints * 2];
	if(!in)
		goto Exit;

	for(int i=0; i<numPoints; i++)
	{
		in[i*2] = controlPoints[i].x;
		in[i*2+1] = controlPoints[i].y;
	}
	
	numSplinePoints = controlPoints[numPoints-1].x - controlPoints[0].x;
	out = new double [numSplinePoints * 2];
	if(!out)
		goto Exit;

	if(!getCubicSpline(out, numSplinePoints, in, numPoints))
		goto Exit;

	splinePoints = new POINT [numSplinePoints];
	if(!splinePoints)
		goto Exit;

	for(int i=0; i<numSplinePoints; i++)
	{
		splinePoints[i].x = (LONG)out[2*i];
		splinePoints[i].y = (LONG)out[2*i + 1];
	}

	numPoints = numSplinePoints;
Exit:
	if(in)
		delete [] in;
	if(out)
		delete [] out;
	return splinePoints;;
}

void CGraph::DrawGraph(HDC hDC)
{
	HPEN hpen = CreatePen(PS_SOLID, 1, m_LineColor);
	RECT rect;
   int i;
   int numPoints;

	GetClipBox(hDC, &rect);
	
	int width = rect.right-rect.left;
	int height = rect.bottom - rect.top;

   int last = (m_nHead-1) % m_nPoints;
   int x0 = -1;

   if(m_nHead > m_nTail)
   {
      for(i=m_nTail, numPoints=0; i<m_nHead; i++)
      {
         ScalePoints(x0, i, numPoints, last, width, height);
      }
   }
   else
   {
      for(i=m_nTail, numPoints=0; i<m_nPoints; i++)
      {
         ScalePoints(x0, i, numPoints, last, width, height);
      }
      for(i=0; i<m_nHead; i++)
      {
         ScalePoints(x0, i, numPoints, last, width, height);
      }
   }

   if(numPoints>0)
   {
      SelectObject(hDC, hpen);
      MoveToEx(hDC, 0,0, 0);
      //PutInMinMaxRange(m_pPoints, numPoints, height);

      //TODO: do the spline curve
      int numPoints2 = numPoints;
      POINT* splinePoints = SplineFromControlPoints(m_pPoints, numPoints2);
      if(splinePoints)
      {
         SubHeightFromYValue(splinePoints, numPoints2, height);
         Polyline(hDC, splinePoints, numPoints2);
         delete [] splinePoints;
      }
      else
      {
         SubHeightFromYValue(m_pPoints, numPoints, height);
         Polyline(hDC, m_pPoints, numPoints);
         //PolyBezier(hDC, m_pPoints, numPoints);
      }
      DeleteObject(hpen);
   }

}
