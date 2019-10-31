// Circle.cpp
// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

// Circle.cpp
// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Circle.h"

CircleOrLine::CircleOrLine(const Point& p0, const Point& p1, const Point& p2)
{
	// from TangentCircles in http://code.google.com/p/heekscad/source/browse/trunk/src/Geom.cpp

	// set default values, in case this fails
	m_radius = 0.0;
	m_c = Point(0, 0);

	if (p0 == p2)
	{
		if (p1 == p0)
		{
			m_is_a_line = true;
			m_p0 = p0;
			m_p1 = p0;
		}
		else
		{
			m_is_a_line = false;
			m_c = (p0 + p1) * 0.5;
			m_radius = p0.dist(m_c);
		}
		return;
	}

	geoff_geometry::CLine line(geoff_geometry::Point(p0.x, p0.y), geoff_geometry::Point(p2.x, p2.y));
	if(fabs(line.Dist(geoff_geometry::Point(p1.x, p1.y))) <= Point::tolerance)
	{
		m_is_a_line = true;
		m_p0 = p0;
		m_p1 = p2;
		return;
	}

	m_is_a_line = false;

	double x1 = p0.x;
	double y1 = p0.y;
	double x2 = p1.x;
	double y2 = p1.y;
	double x3 = p2.x;
	double y3 = p2.y;

	double a = 2 * (x1 - x2);
	double b = 2 * (y1 - y2);
	double d = (x1 * x1 + y1 * y1) - (x2 * x2 + y2 * y2);

	double A = 2 * (x1 - x3);
	double B = 2 * (y1 - y3);
	double D = (x1 * x1 + y1 * y1) - (x3 * x3 + y3 * y3);

	double aBmbA = (a*B - b * A); // aB - bA

	// x = k + Kr where
	double k = (B*d - b * D) / aBmbA;

	// y = l + Lr where
	double l = (-A * d + a * D) / aBmbA;

	double qa = -1;
	double qb = 0.0;
	double qc = k * k + x1 * x1 - 2 * k*x1 + l * l + y1 * y1 - 2 * l*y1;

	// solve the quadratic equation, r = (-b +- sqrt(b*b - 4*a*c))/(2 * a)
	for (int qs = 0; qs < 2; qs++) {
		double bb = qb * qb;
		double ac4 = 4 * qa*qc;
		if (ac4 <= bb) {
			double r = (-qb + ((qs == 0) ? 1 : -1) * sqrt(bb - ac4)) / (2 * qa);
			double x = k;
			double y = l;

			// set the circle
			if (r >= 0.0) {
				m_c = Point(x, y);
				m_radius = r;
			}
		}
	}
}

bool CircleOrLine::PointIsOn(const Point& p, double accuracy)
{
	if (m_is_a_line)
	{
		geoff_geometry::CLine line(geoff_geometry::Point(m_p0.x, m_p0.y), geoff_geometry::Point(m_p1.x, m_p1.y));
		return (fabs(line.Dist(geoff_geometry::Point(p.x, p.y))) <= accuracy);
	}
	double rp = p.dist(m_c);
	bool on = fabs(m_radius - rp) < accuracy;
	return on;
}

bool CircleOrLine::LineIsOn(const Point& p0, const Point& p1, double accuracy)
{
	// checks the points are on the arc, to the given accuracy, and the mid point of the line.

	if (this->m_is_a_line)
	{
		if (!PointIsOn(p0, accuracy))return false;
		if (!PointIsOn(p1, accuracy))return false;

		Point this_dir = m_p1 - m_p0;
		this_dir.normalize();
		Point dir = p1 - p0;
		dir.normalize();
		return ((dir * this_dir) >= -0.0000000001); // they are going in the same direction
	}

	if (!PointIsOn(p0, accuracy))return false;
	if (!PointIsOn(p1, accuracy))return false;

	Point mid = Point((p0 + p1) / 2);
	if (!PointIsOn(mid, accuracy))return false;

	return true;
}