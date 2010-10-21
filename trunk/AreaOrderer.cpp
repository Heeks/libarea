// AreaOrderer.cpp
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "AreaOrderer.h"
#include "Area.h"

CAreaOrderer::CAreaOrderer()
{
}
	
eOverlapType CAreaOrderer::GetOverlapType(const CArea& a1, const CArea& a2)const
{
	CArea temp_a1(a1);
	temp_a1.Subtract(a2);
	if(temp_a1.m_curves.size() == 0)
	{
		return eInside;
	}

	CArea temp_a2(a2);
	temp_a2.Subtract(a1);
	if(temp_a2.m_curves.size() == 0)
	{
		return eOutside;
	}

	return eSiblings;
}

void CAreaOrderer::Insert(const CCurve* pcurve)
{
	CArea a1;
	CCurve curve(*pcurve);
	if(curve.IsClockwise())curve.Reverse();
	a1.m_curves.push_back(curve);

	for(std::list<CArea>::iterator It = m_areas.begin(); It != m_areas.end(); It++)
	{
		CArea& a2 = *It;

		switch(GetOverlapType(a1, a2))
		{
		case eOutside:
			{
				// insert a new area, before this one
				m_areas.insert(It, a1);
				printf("outside\n");
				return;
			}
			break;

		case eInside:
				printf("inside\n");
			// continue to the next area
			break;

		case eSiblings:
			{
				printf("siblings\n");
				// add the curve to this area
				a2.m_curves.push_back(curve);
				return;
			}
			break;
		}
	}

	// only gets here if the curve is inside all the existing areas

	// add a new area
	m_areas.push_back(a1);
}

CArea CAreaOrderer::ResultArea()const
{
	CArea a;
	bool outside = true;

	for(std::list<CArea>::const_iterator It = m_areas.begin(); It != m_areas.end(); It++)
	{
		const CArea& a2 = *It;

		for(std::list<CCurve>::const_iterator CIt = a2.m_curves.begin(); CIt != a2.m_curves.end(); CIt++)
		{
			const CCurve& c = *CIt;
			a.m_curves.push_back(c);
			if(!outside)a.m_curves.back().Reverse();			
		}
		outside = !outside;
	}

	return a;
}

