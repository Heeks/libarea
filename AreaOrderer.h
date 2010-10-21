// AreaOrderer.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once
#include <list>

class CArea;
class CCurve;

enum eOverlapType
{
	eOutside,
	eInside,
	eSiblings,
};

class CAreaOrderer
{
	std::list<CArea> m_areas; // ordered areas, outside first, containing only anti-clockwise curves
	
	eOverlapType GetOverlapType(const CArea& a1, const CArea& a2)const;

public:
	CAreaOrderer();

	void Insert(const CCurve* pcurve);
	CArea ResultArea()const;
};