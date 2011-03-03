// Arc.cpp

// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Arc.h"
#include "Curve.h"

void Arc::SetDirWithPoint(const Point& p)
{
	double angs = atan2(m_s.y - m_c.y, m_s.x - m_c.x);
	double ange = atan2(m_e.y - m_c.y, m_e.x - m_c.x);
	double angp = atan2(p.y - m_c.y, p.x - m_c.x);
	if(ange < angs)ange += 6.2831853071795864;
	if(angp < angs - 0.0000000000001)angp += 6.2831853071795864;
	if(angp > ange + 0.0000000000001)m_dir = false;
	else m_dir = true;
}

double Arc::IncludedAngle()const
{
	double angs = atan2(m_s.y - m_c.y, m_s.x - m_c.x);
	double ange = atan2(m_e.y - m_c.y, m_e.x - m_c.x);
	if(m_dir)
	{
		// make sure ange > angs
		if(ange < angs)ange += 6.2831853071795864;
	}
	else
	{
		// make sure angs > ange
		if(angs < ange)angs += 6.2831853071795864;
	}

	return fabs(ange - angs);
}

bool Arc::AlmostALine()const
{
	Point mid_point = MidParam(0.5);
	if(Line(m_s, m_e - m_s).Dist(mid_point) <= Point::tolerance)
		return true;

	const double max_arc_radius = 1.0 / Point::tolerance;
	double radius = m_c.dist(m_s);
	if (radius > max_arc_radius)
	{
		return true;	// We don't want to produce an arc whose radius is too large.
	}

	return false;
}

Point Arc::MidParam(double param)const {
	/// returns a point which is 0-1 along arc
	if(fabs(param) < 0.00000000000001)return m_s;
	if(fabs(param - 1.0) < 0.00000000000001)return m_e;

	Point p;
	Point v = m_s - m_c;
	v.Rotate(param * IncludedAngle());
	p = v + m_c;

	return p;
}