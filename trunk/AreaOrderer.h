// AreaOrderer.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once
#include <list>
#include <set>

class CArea;
class CCurve;

enum eOverlapType
{
	eOutside,
	eInside,
	eSiblings,
	eCrossing,
};

class CAreaOrderer;

class CInnerCurves
{
	CInnerCurves* m_pOuter;
	const CCurve* m_curve; // always empty if top level
	std::set<CInnerCurves*> m_inner_curves;

	eOverlapType GetOverlapType(const CCurve* c1, const CCurve* c2)const;

public:
	static CAreaOrderer* area_orderer;
	CInnerCurves(CInnerCurves* pOuter, const CCurve* curve);

	void Insert(const CCurve* pcurve);
	void GetArea(CArea &area, bool outside);
};

class CAreaOrderer
{
public:
	CInnerCurves* m_top_level;

	CAreaOrderer();

	void Insert(CCurve* pcurve);
	CArea ResultArea()const;
};