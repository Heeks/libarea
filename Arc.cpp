// Arc.cpp

// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Arc.h"

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

double Arc::IncludedAngle()
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