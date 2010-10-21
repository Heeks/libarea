// AreaOrderer.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "AreaOrderer.h"
#include "Area.h"

CAreaOrderer* CInnerCurves::area_orderer = NULL;

CInnerCurves::CInnerCurves(CInnerCurves* pOuter, const CCurve* curve)
{
	m_pOuter = pOuter;
	m_curve = curve;
}

void CInnerCurves::Insert(const CCurve* pcurve)
{
	std::list<CInnerCurves*> outside_of_these;

	// check all inner curves
	for(std::set<CInnerCurves*>::iterator It = m_inner_curves.begin(); It != m_inner_curves.end(); It++)
	{
		CInnerCurves* c = *It;

		switch(GetOverlapType(pcurve, c->m_curve))
		{
		case eOutside:
			outside_of_these.push_back(c);
			break;

		case eInside:
			// insert in this inner curve
			c->Insert(pcurve);
			return;

		default:
			break;
		}
	}

	// add as a new inner
	CInnerCurves* new_item = new CInnerCurves(this, pcurve);
	this->m_inner_curves.insert(new_item);

	for(std::list<CInnerCurves*>::iterator It = outside_of_these.begin(); It != outside_of_these.end(); It++)
	{
		// move items
		CInnerCurves* c = *It;
		c->m_pOuter = new_item;
		new_item->m_inner_curves.insert(c);
		this->m_inner_curves.erase(c);
	}
}

eOverlapType CInnerCurves::GetOverlapType(const CCurve* c1, const CCurve* c2)const
{
	CArea a1;
	a1.m_curves.push_back(*c1);
	CArea a2;
	a2.m_curves.push_back(*c2);
	a1.Subtract(a2);
	if(a1.m_curves.size() == 0)
	{
		return eInside;
	}

	a1.m_curves.clear();
	a1.m_curves.push_back(*c1);
	a2.Subtract(a1);
	if(a2.m_curves.size() == 0)
	{
		return eOutside;
	}

	a2.m_curves.clear();
	a2.m_curves.push_back(*c2);
	a1.Intersect(a2);
	if(a1.m_curves.size() == 0)
	{
		return eSiblings;
	}

	return eCrossing;
}

void CInnerCurves::GetArea(CArea &area, bool outside)
{
	for(std::set<CInnerCurves*>::iterator It = m_inner_curves.begin(); It != m_inner_curves.end(); It++)
	{
		CInnerCurves* c = *It;
		area.m_curves.push_back(*c->m_curve);
		if(!outside)area.m_curves.back().Reverse();
		c->GetArea(area, !outside);
	}
}

CAreaOrderer::CAreaOrderer()
{
	m_top_level = new CInnerCurves(NULL, NULL);
}

void CAreaOrderer::Insert(CCurve* pcurve)
{
	CInnerCurves::area_orderer = this;

	// make them all anti-clockwise as they come in
	if(pcurve->IsClockwise())pcurve->Reverse();

	m_top_level->Insert(pcurve);
}

CArea CAreaOrderer::ResultArea()const
{
	CArea a;

	if(m_top_level)
	{
		m_top_level->GetArea(a, true);
	}

	return a;
}

