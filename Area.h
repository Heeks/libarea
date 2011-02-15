// Area.h
// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#ifndef AREA_HEADER
#define AREA_HEADER

#include "Curve.h"

struct CAreaPocketParams
{
	double tool_radius;
	double extra_offset;
	double stepover;
	bool from_center;
	bool use_zig_zag;
	double zig_angle;
	bool only_cut_first_offset;
	CAreaPocketParams(double Tool_radius, double Extra_offset, double Stepover, bool From_center, bool Use_zig_zag, double Zig_angle, bool Only_cut_first_offset)
	{
		tool_radius = Tool_radius;
		extra_offset = Extra_offset;
		stepover = Stepover;
		from_center = From_center;
		use_zig_zag = Use_zig_zag;
		zig_angle = Zig_angle;
		only_cut_first_offset = Only_cut_first_offset;
	}
};

class CArea
{
public:
	std::list<CCurve> m_curves;
	static double m_accuracy;
	static double m_units; // 1.0 for mm, 25.4 for inches. All points are multiplied by this before going to the engine
	static bool m_fit_arcs;
	static double m_processing_done; // 0.0 to 100.0, set inside MakeOnePocketCurve
	static double m_single_area_processing_length;
	static double m_after_MakeOffsets_length;
	static double m_MakeOffsets_increment;
	static double m_split_processing_length;
	static bool m_set_processing_length_in_split;
	static bool m_please_abort; // the user sets this from another thread, to tell MakeOnePocketCurve to finish with no result.

	void append(const CCurve& curve);
	void Subtract(const CArea& a2);
	void Intersect(const CArea& a2);
	void Union(const CArea& a2);
	void Offset(double inwards_value);
	void FitArcs();
	unsigned int num_curves(){return m_curves.size();}
	Point NearestPoint(const Point& p)const;
	void GetBox(CBox &box);
	void Reorder();
	void MakePocketToolpath(std::list<CCurve> &toolpath, const CAreaPocketParams &params)const;
	void SplitAndMakePocketToolpath(std::list<CCurve> &toolpath, const CAreaPocketParams &params)const;
	void MakeOnePocketCurve(CCurve& curve, const CAreaPocketParams &params)const;
	bool HolesLinked()const;
	void Split(std::list<CArea> &m_areas)const;
	double GetArea(bool always_add = false)const;
};

enum eOverlapType
{
	eOutside,
	eInside,
	eSiblings,
	eCrossing,
};

eOverlapType GetOverlapType(const CCurve& c1, const CCurve& c2);
eOverlapType GetOverlapType(const CArea& a1, const CArea& a2);
bool IsInside(const Point& p, const CCurve& c);
bool IsInside(const Point& p, const CArea& a);

#endif // #define AREA_HEADER
