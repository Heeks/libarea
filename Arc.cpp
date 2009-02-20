// Arc.cpp

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
