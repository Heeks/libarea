// Arc.h

#pragma once

#include "Point.h"

class Arc{
public:
	Point m_s;
	Point m_e;
	Point m_c;
	bool m_dir; // true - anti-clockwise, false - clockwise

	Arc():m_dir(true){}
	Arc(const Point& s, const Point& e, const Point& c, bool dir):m_s(s), m_e(e), m_c(c), m_dir(dir){}

	void SetDirWithPoint(const Point& p); // set m_dir, such that this point lies between m_s and m_e
};