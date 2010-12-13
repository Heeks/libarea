// Line.cpp

#include "Line.h"

Line::Line(const Point& P0, const Point& V):p0(P0), v(V)
{
}

double Line::Dist(const Point& p)const
{
	Point vn = v;
	vn.normalize();
	double d1 = p0 * vn;
	double d2 = p * vn;
	Point pn = p0 + vn * (d2 - d1);

	return pn.dist(p);
}
