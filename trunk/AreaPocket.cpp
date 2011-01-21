// AreaPocket.cpp

// implements CArea::MakeOnePocketCurve

#include "Area.h"

#include <map>

static const CAreaPocketParams* pocket_params = NULL;

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
	if(CArea::m_please_abort)return;
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
			Span span(*prev_p, vertex);

			// order inners on this span
			std::multimap<double, CurveTree*> ordered_inners;
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
				if(CArea::m_please_abort)return;
			}

			if(CArea::m_please_abort)return;
			for(std::multimap<double, CurveTree*>::iterator It2 = ordered_inners.begin(); It2 != ordered_inners.end(); It2++)
			{
				CurveTree& inner = *(It2->second);
				if(inner.point_on_parent != output.m_vertices.back().m_p)
				{
					output.m_vertices.push_back(CVertex(vertex.m_type, inner.point_on_parent, vertex.m_c));
				}
				if(CArea::m_please_abort)return;
				inner.GetCurve(output);
				output.m_vertices.push_back(CVertex(inner.point_on_parent));
			}

			if(output.m_vertices.back().m_p != vertex.m_p)output.m_vertices.push_back(vertex);
		}
		prev_p = &vertex.m_p;
	}

	if(CArea::m_please_abort)return;
	for(std::list<CurveTree*>::iterator It2 = inners_to_visit.begin(); It2 != inners_to_visit.end(); It2++)
	{
		CurveTree &inner = *(*It2);
		if(inner.point_on_parent != output.m_vertices.back().m_p)
		{
			output.m_vertices.push_back(CVertex(inner.point_on_parent));
		}
		if(CArea::m_please_abort)return;
		inner.GetCurve(output);
		output.m_vertices.push_back(CVertex(inner.point_on_parent));
	}
}

void CurveTree::MakeOffsets()
{
	// make offsets

	if(CArea::m_please_abort)return;
	CArea smaller;
	smaller.m_curves.push_back(curve);
	smaller.Offset(pocket_params->stepover);

	if(CArea::m_please_abort)return;

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
			if(CArea::m_please_abort)return;
			Point island_point = island_and_offset->island->NearestPoint(inners.back().point_on_parent);
			if(CArea::m_please_abort)return;
			inners.back().curve.ChangeStart(island_point);
			if(CArea::m_please_abort)return;
			smaller.Subtract(island_and_offset->offset);
			if(CArea::m_please_abort)return;
			It = offset_islands.erase(It);
			if(offset_islands.size() == 0)break;
			It = offset_islands.begin();
		}
	}

	CArea::m_processing_done += CArea::m_MakeOffsets_increment;
	if(CArea::m_processing_done > CArea::m_after_MakeOffsets_length)CArea::m_processing_done = CArea::m_after_MakeOffsets_length;

	std::list<CArea> separate_areas;
	smaller.Split(separate_areas);
	if(CArea::m_please_abort)return;
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
			if(CArea::m_please_abort)return;
		}

		inners.back().point_on_parent = curve.NearestPoint(first_curve);
		if(CArea::m_please_abort)return;
		Point first_curve_point = first_curve.NearestPoint(inners.back().point_on_parent);
		if(CArea::m_please_abort)return;
		inners.back().curve.ChangeStart(first_curve_point);
		if(CArea::m_please_abort)return;
		inners.back().MakeOffsets(); // recursive
		if(CArea::m_please_abort)return;
	}
}

void CArea::MakeOnePocketCurve(CCurve& curve, const CAreaPocketParams &params)const
{
	if(CArea::m_please_abort)return;
	pocket_params = &params;
	if(m_curves.size() == 0)
	{
		CArea::m_processing_done += CArea::m_single_area_processing_length;
		return;
	}
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
			if(m_please_abort)return;
		}
	}

	CArea::m_processing_done += CArea::m_single_area_processing_length * 0.1;

	double MakeOffsets_processing_length = CArea::m_single_area_processing_length * 0.8;
	CArea::m_after_MakeOffsets_length = CArea::m_processing_done + MakeOffsets_processing_length;
	double guess_num_offsets = sqrt(GetArea(true)) * 0.5 / params.stepover;
	CArea::m_MakeOffsets_increment = MakeOffsets_processing_length / guess_num_offsets;

	top_level.MakeOffsets();
	if(CArea::m_please_abort)return;
	CArea::m_processing_done = CArea::m_after_MakeOffsets_length;


	top_level.GetCurve(curve);
	CArea::m_processing_done += CArea::m_single_area_processing_length * 0.1;
}

