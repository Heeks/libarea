// Circle.h
// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "Point.h"


class CircleOrLine {
public:
	bool m_is_a_line; // true for line, false for circle
	Point m_c;       // just for circle
	double m_radius;  // just for circle
	Point m_p0;       // just for line
	Point m_p1;       // just for line

	CircleOrLine(const Point& c, double radius) :m_c(c), m_radius(radius), m_is_a_line(false) {}
	CircleOrLine(const Point& p0, const Point& p1, const Point& p2); // circle through three points

	bool PointIsOn(const Point& p, double accuracy);
	bool LineIsOn(const Point& p0, const Point& p1, double accuracy);
};