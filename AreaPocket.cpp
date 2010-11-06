// AreaPocket.cpp

// implements CArea::MakeOnePocketCurve

#include "Area.h"

#include <map>

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

class IslandAndOffset
{
public:
	const CCurve* island;
	CArea offset;
	IslandAndOffset(const CCurve* Island)
	{
		island = Island;
		offset.m_curves.push_back(*island);
		offset.m_curves.back().Reverse();
		offset.Offset(-pocket_params->stepover);
	}
};

class CurveTree
{
public:
	Point point_on_parent;
	CCurve curve;
	std::list<CurveTree> inners;
	std::list<const IslandAndOffset*> offset_islands;
	CurveTree(const CCurve &c)
	{
		curve = c;
	}

	void MakeOffsets();
	void GetCurve(CCurve& curve);
};

void CurveTree::GetCurve(CCurve& output)
{
	// walk around the curve adding spans to output until we get to an inner's point_on_parent
	// then add a line from the inner's point_on_parent to inner's start point, then GetCurve from inner

	// add start point
	output.m_vertices.push_back(CVertex(curve.m_vertices.front()));

	std::list<CurveTree*> inners_to_visit;
	for(std::list<CurveTree>::iterator It2 = inners.begin(); It2 != inners.end(); It2++)
	{
		inners_to_visit.push_back(&(*It2));
	}

	const Point* prev_p = NULL;
	for(std::list<CVertex>::iterator It = curve.m_vertices.begin(); It != curve.m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p)
		{
			SpanPtr span(*prev_p, vertex);

			// order inners on this span
			std::map<double, CurveTree*> ordered_inners;
			for(std::list<CurveTree*>::iterator It2 = inners_to_visit.begin(); It2 != inners_to_visit.end();)
			{
				CurveTree *inner = *It2;
				double t;
				if(span.On(inner->point_on_parent, &t))
				{
					ordered_inners.insert(std::make_pair(t, inner));
					It2 = inners_to_visit.erase(It2);
				}
				else
				{
					It2++;
				}
			}

			for(std::map<double, CurveTree*>::iterator It2 = ordered_inners.begin(); It2 != ordered_inners.end(); It2++)
			{
				CurveTree& inner = *(It2->second);
				if(inner.point_on_parent != output.m_vertices.back().m_p)
				{
					output.m_vertices.push_back(CVertex(vertex.m_type, inner.point_on_parent, vertex.m_c));
				}
				inner.GetCurve(output);
				output.m_vertices.push_back(CVertex(inner.point_on_parent));
			}

			if(output.m_vertices.back().m_p != vertex.m_p)output.m_vertices.push_back(vertex);
		}
		prev_p = &vertex.m_p;
	}

	for(std::list<CurveTree*>::iterator It2 = inners_to_visit.begin(); It2 != inners_to_visit.end(); It2++)
	{
		CurveTree &inner = *(*It2);
		if(inner.point_on_parent != output.m_vertices.back().m_p)
		{
			output.m_vertices.push_back(CVertex(inner.point_on_parent));
		}
		inner.GetCurve(output);
		output.m_vertices.push_back(CVertex(inner.point_on_parent));
	}
}

void CurveTree::MakeOffsets()
{
	// make offsets

	CArea smaller;
	smaller.m_curves.push_back(curve);
	smaller.Offset(pocket_params->stepover);

	// test islands
	for(std::list<const IslandAndOffset*>::iterator It = offset_islands.begin(); It != offset_islands.end();)
	{
		const IslandAndOffset* island_and_offset = *It;

		if(GetOverlapType(island_and_offset->offset, smaller) == eInside)
			It++; // island is still inside
		else
		{
			inners.push_back(*island_and_offset->island);
			inners.back().point_on_parent = curve.NearestPoint(*island_and_offset->island);
			Point island_point = island_and_offset->island->NearestPoint(inners.back().point_on_parent);
			inners.back().curve.ChangeStart(island_point);
			smaller.Subtract(island_and_offset->offset);
			It = offset_islands.erase(It);
		}
	}

	std::list<CArea> separate_areas;
	smaller.Split(separate_areas);
	for(std::list<CArea>::iterator It = separate_areas.begin(); It != separate_areas.end(); It++)
	{
		CArea& separate_area = *It;
		CCurve& first_curve = separate_area.m_curves.front();
		inners.push_back(first_curve);

		for(std::list<const IslandAndOffset*>::iterator It = offset_islands.begin(); It != offset_islands.end();It++)
		{
			const IslandAndOffset* island_and_offset = *It;
			if(GetOverlapType(island_and_offset->offset, separate_area) == eInside)
				inners.back().offset_islands.push_back(island_and_offset);
		}

		inners.back().point_on_parent = curve.NearestPoint(first_curve);
		Point first_curve_point = first_curve.NearestPoint(inners.back().point_on_parent);
		inners.back().curve.ChangeStart(first_curve_point);
		inners.back().MakeOffsets(); // recursive
	}
}

void CArea::MakeOnePocketCurve(CCurve& curve, const CAreaPocketParams &params)const
{
	arealist_for_pocket.clear();
	pocket_params = &params;
	if(m_curves.size() == 0)return;
	CurveTree top_level(m_curves.front());

	std::list<IslandAndOffset> offset_islands;

	for(std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		const CCurve& c = *It;
		if(It != m_curves.begin())
		{
			IslandAndOffset island_and_offset(&c);
			offset_islands.push_back(island_and_offset);
			top_level.offset_islands.push_back(&(offset_islands.back()));
		}
	}

	top_level.MakeOffsets();

	top_level.GetCurve(curve);
}

