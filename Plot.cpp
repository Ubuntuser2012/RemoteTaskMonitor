#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include "Plot.h"
extern "C" {
#include "../cubicspline/cubicspline.h"
}
#define GRAPHLINE_COLOR 0x0000FF00

#include "../common/util.h"

// Polynomial interpolation - Spline
// http://en.wikipedia.org/wiki/Polynomial_interpolation
// p(x) = Sum[i=0 to n]( y(i) . Product[j=0 to n,j!=i]( (x - x(j)) / (x(i) - x(j) ) ) )


NUMBER PLOT::ProductForPolynomialInterpolationOfX(int i, NUMBER x, int numControlPointsLessOne, const POINT* controlPoints)
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

NUMBER PLOT::PolynomialInterpolationOfX(NUMBER x, int numControlPointsLessOne, const POINT* controlPoints)
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

PLOT* PLOT::create() 
{
   PLOT* o = new PLOT;
   if(!o)
      throw 0;
   o->m_ref++;
   return o;
}

PLOT::PLOT()
{
	m_LineColor = GRAPHLINE_COLOR;
	m_control_points = NULL;
	m_nPoints = 0;
	m_nHead = 0;
	m_nTail = 0;
	m_x=0;
	m_y=0;
}

PLOT::~PLOT()
{
	if(m_control_points)
		free(m_control_points);
	if(m_x)
		free(m_x);
	if(m_y)
		free(m_y);
}

long PLOT::InitG(long id, COLORREF LineColor, int maxY, RECT *window)//long nPoints, unsigned int maxY)
{
	m_id=id;
	m_LineColor = LineColor;
	m_maxY = maxY;
	m_nPoints = (window->right - window->left)/10;
	m_control_points = (POINT*) malloc(m_nPoints * sizeof(POINT));
	memset(m_control_points, 0, m_nPoints * sizeof(POINT));
	m_x = (int *) malloc(m_nPoints * sizeof(int));
	m_y = (int*) malloc(m_nPoints * sizeof(int));
	memset(m_x, 0, m_nPoints * sizeof(int));
	memset(m_y, 0, m_nPoints * sizeof(int));

	m_nHead = m_nPoints - 1;
	m_nTail = 0;

	for (int i = 0; i < m_nPoints; i++)
	{
		m_x[i] = i;
		m_y[i] = 0;
	}

	return (m_nPoints && m_x && m_y) ? 1: 0;
}

void PLOT::AddPoint(int x, int y)
{
	if(m_nPoints)//que max size is not zero
	{
		m_nHead = (m_nHead + 1) % m_nPoints;
		m_nTail = (m_nTail + 1) % m_nPoints;

		m_x[m_nHead] = x + m_nPoints ;
		m_y[m_nHead] = y;
	}
}

void PLOT::SubHeightFromYValue(POINT* points, int numPoints, int height)
{
   for(int i=0; i<numPoints; i++)
   {
      int y = height - points[i].y;
      points[i].y = y;
   }
}

void PLOT::PutInMinMaxRange(POINT* points, int numPoints, int height)
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

POINT* PLOT::SplineFromControlPoints(POINT* controlPoints, int &numPoints)
{
	POINT *splinePoints = 0;
	int numSplinePoints;
	double *out = 0;
	double *in = new double [numPoints * 2];
	if(!in)
		goto Exit;

	for(int i=0; i<numPoints; i++)
	{
		in[i*2] = controlPoints[i].x;
		in[i*2+1] = controlPoints[i].y;
	}
	
	numSplinePoints = controlPoints[numPoints-1].x - controlPoints[0].x;
	if (numSplinePoints < 1)
		goto Exit;
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
	return splinePoints;
}

void PLOT::ScalePoints(int width, int height)
{
	int maxX = m_x[m_nHead] - m_x[m_nTail];
	int minX = m_x[m_nTail];
	if (m_nHead > m_nTail)
	{
		int i, j;
		for (i = m_nTail, j = 0; i <= m_nHead; i++, j++)
		{
			m_control_points[j].x = (m_x[i] - minX) * width / maxX;
			m_control_points[j].y = m_y[i] * height / m_maxY;
		}
	}
	else
	{
		int i, j;
		for (i = m_nTail, j = 0; i<m_nPoints; i++, j++)
		{
			m_control_points[j].x = (m_x[i] - minX) * width / maxX;
			m_control_points[j].y = m_y[i] * height / m_maxY;
		}
		for (i = 0; i <= m_nHead; i++, j++)
		{
			m_control_points[j].x = (m_x[i] - minX) * width / maxX;
			m_control_points[j].y = m_y[i] * height / m_maxY;
		}
	}
}

void PLOT::DrawGraph(HDC hDC)
{
	HPEN hpen = CreatePen(PS_SOLID, 1, m_LineColor);
	RECT rect;
   int numPoints;

	GetClipBox(hDC, &rect);
	
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	ScalePoints(width, height);

   {
      SelectObject(hDC, hpen);
      MoveToEx(hDC, 0,0, 0);
      //PutInMinMaxRange(m_pPoints, numPoints, height);

      //TODO: do the spline curve
      int numPoints2 = m_nPoints;
      POINT* splinePoints = SplineFromControlPoints(m_control_points, numPoints2);

	  if(splinePoints)
      {
         SubHeightFromYValue(splinePoints, numPoints2, height);
         Polyline(hDC, splinePoints, numPoints2);
         delete [] splinePoints;
      }
      else
      {
         SubHeightFromYValue(m_control_points, numPoints2, height);
         Polyline(hDC, m_control_points, numPoints2);
         //PolyBezier(hDC, m_pPoints, numPoints);
      }
      DeleteObject(hpen);
   }

}
