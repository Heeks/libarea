// Area.h

// written by Dan Heeks, February 6th 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#ifndef AREA_HEADER
#define AREA_HEADER

#include <vector>
#include <list>
#include <math.h>
#include "Point.h"
#include "Box.h"

#ifdef CLIPPER_NOT_KBOOL
#include "clipper.hpp"
#endif

class Bool_Engine;
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
};

class SpanPtr
{
public:
	const Point& m_p;
	const CVertex& m_v;
	SpanPtr(const Point& p, const CVertex& v):m_p(p), m_v(v){}
	Point NearestPoint(const Point& p);
	void GetBox(CBox &box);
	double IncludedAngle()const;
	double GetArea()const;
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
	Point NearestPoint(const Point& p);
	void GetBox(CBox &box);
	void Reverse();
	double GetArea()const;
	bool IsClockwise()const{return GetArea()>0;}
};

enum resultType{
	rtAll,
	rtClockwise,
	rtAntiClockwise,
};

class CArea
{
#ifdef CLIPPER_NOT_KBOOL
	void MakePolyPoly( clipper::TPolyPolygon &pp )const;
	void SetFromResult( const clipper::TPolyPolygon& pp, resultType result_type = rtAll );
	void AddVertex(std::list<clipper::TDoublePoint> &pts, const CVertex& vertex, const CVertex* prev_vertex = NULL)const;
	void OffsetWithLoops(const clipper::TPolyPolygon &pp, clipper::TPolyPolygon &pp_new, double inwards_value)const;
	void MakeLoop(const clipper::TDoublePoint &pt0, const clipper::TDoublePoint &pt1, const clipper::TDoublePoint &pt2, std::list<clipper::TDoublePoint> &pts, double radius)const;
#else
	void MakeGroup( Bool_Engine* booleng, bool a_not_b )const;
	void SetFromResult( Bool_Engine* booleng );
	void AddVertex(Bool_Engine* booleng, const CVertex& vertex, const CVertex* prev_vertex = NULL)const;
#endif

public:
	std::list<CCurve> m_curves;
	static double m_round_corners_factor; // 1.0 for round 90 degree corners, 1.5 for square 90 degree corners
	static double m_accuracy;
	static double m_units; // 1.0 for mm, 25.4 for inches. All points are multiplied by this before going to the engine

#ifdef CLIPPER_NOT_KBOOL
#else
	static void ArmBoolEng( Bool_Engine* booleng );
#endif

	void append(const CCurve& curve);
	void Subtract(const CArea& a2);
	void Intersect(const CArea& a2);
	void Union(const CArea& a2);
	void Offset(double inwards_value);
	void FitArcs();
	unsigned int num_curves(){return m_curves.size();}
	Point NearestPoint(const Point& p);
	void GetBox(CBox &box);
	void Reorder();
};

#endif // #define AREA_HEADER
