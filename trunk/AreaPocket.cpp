// AreaPocket.cpp

// implements CArea::MakeOnePocketCurve

#include "Area.h"

static std::list<CArea> arealist_for_pocket;
static const CAreaPocketParams* pocket_params = NULL;

static void recur(const CArea &a1){
    // this makes arealist by recursively offsetting a1 inwards
    
    if(a1.m_curves.size() == 0)
        return;
    
	if(pocket_params->from_center)
        arealist_for_pocket.push_front(a1);
    else
        arealist_for_pocket.push_back(a1);

    CArea a_offset(a1);
	a_offset.Offset(pocket_params->stepover);
    
    // split curves into new areas
	std::list<CArea> m_areas;
	a_offset.Split(m_areas);

	for(std::list<CArea>::iterator It = m_areas.begin(); It != m_areas.end(); It++)
	{
		CArea &a2 = *It;
		recur(a2);
	}
}

static void get_curve(const std::list<CArea> &arealist, CCurve &curve)
{
	for(std::list<CArea>::const_iterator It = arealist.begin(); It != arealist.end(); It++)
	{
		const CArea& a = *It;
		for(std::list<CCurve>::const_iterator CIt = a.m_curves.begin(); CIt != a.m_curves.end(); CIt++)
		{
			const CCurve& c = *CIt;
			for(std::list<CVertex>::const_iterator VIt = c.m_vertices.begin(); VIt != c.m_vertices.end(); VIt++)
			{
				const CVertex& v = *VIt;
				curve.m_vertices.push_back(v);
			}
		}
	}
}

void CArea::MakeOnePocketCurve(CCurve& curve, const CAreaPocketParams &params)const
{
	arealist_for_pocket.clear();
	pocket_params = &params;
	recur(*this);
    get_curve(arealist_for_pocket, curve);
}

