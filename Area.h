// Area.h

// written by Dan Heeks, February 6th 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#ifndef AREA_HEADER
#define AREA_HEADER

#include <vector>
#include <list>
#include <math.h>
#include "Point.h"
#include "Box.h"

class Arc;

class CVertex
{
public:
	int m_type; // 0 - line ( or start point ), 1 - anti-clockwise arc, -1 - clockwise arc
	Point m_p; // end point
	Point m_c; // centre point in absolute coordinates
	int m_user_data;

	CVertex():m_type(0), m_p(Point(0, 0)), m_c(Point(0,0)), m_user_data(0){}
	CVertex(int type, const Point& p, const Point& c, int user_data = 0);
	CVertex(const Point& p, int user_data = 0);
};

class SpanPtr
{
	Point NearestPointNotOnSpan(const Point& p)const;
	double Parameter(const Point& p)const;
	Point NearestPointToSpan(const SpanPtr& p, double &d)const;

public:
	bool m_start_span;
	const Point& m_p;
	const CVertex& m_v;
	SpanPtr(const Point& p, const CVertex& v, bool start_span = false):m_p(p), m_v(v), m_start_span(start_span){}
	Point NearestPoint(const Point& p)const;
	Point NearestPoint(const SpanPtr& p, double *d = NULL)const;
	void GetBox(CBox &box);
	double IncludedAngle()const;
	double GetArea()const;
	bool On(const Point& p, double* t = NULL)const;
	Point MidPerim(double d)const;
	Point MidParam(double param)const;
};

class CCurve
{
	// a closed curve, please make sure you add an end point, the same as the start point

protected:
	bool CheckForArc(const CVertex& prev_vt, std::list<const CVertex*>& might_be_an_arc, Arc &arc);
	void AddArcOrLines(bool check_for_arc, std::list<CVertex> &new_vertices, std::list<const CVertex*>& might_be_an_arc, Arc &arc, bool &arc_found, bool &arc_added);

public:
	std::list<CVertex> m_vertices;
	void append(const CVertex& vertex);

	void FitArcs();
	Point NearestPoint(const Point& p)const;
	Point NearestPoint(const CCurve& p, double *d = NULL)const;
	Point NearestPoint(const SpanPtr& p, double *d = NULL)const;
	void GetBox(CBox &box);
	void Reverse();
	double GetArea()const;
	bool IsClockwise()const{return GetArea()>0;}
	void ChangeStart(const Point &p);
};

struct CAreaPocketParams
{
	double tool_radius;
	double extra_offset;
	double stepover;
	bool from_center;
	bool use_zig_zag;
	double zig_angle;
	CAreaPocketParams(double Tool_radius, double Extra_offset, double Stepover, bool From_center, bool Use_zig_zag, double Zig_angle)
	{
		tool_radius = Tool_radius;
		extra_offset = Extra_offset;
		stepover = Stepover;
		from_center = From_center;
		use_zig_zag = Use_zig_zag;
		zig_angle = Zig_angle;
	}
};

class CArea
{
public:
	std::list<CCurve> m_curves;
	static double m_accuracy;
	static double m_units; // 1.0 for mm, 25.4 for inches. All points are multiplied by this before going to the engine

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
	void MakeOnePocketCurve(CCurve& curve, const CAreaPocketParams &params)const;
	bool HolesLinked();
	void Split(std::list<CArea> &m_areas);
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

#endif // #define AREA_HEADER
