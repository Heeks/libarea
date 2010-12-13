// Line.h

#pragma once

#include "Point.h"

class Line{
public:
	Point p0;
	Point v;

	// constructors
	Line(const Point& P0, const Point& V);

	double Dist(const Point& p)const;
};