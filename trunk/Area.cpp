// Area.cpp

// written by Dan Heeks, February 6th 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#include "Area.h"
#include "kbool/include/_lnk_itr.h"
#include "kbool/include/booleng.h"
#include "Circle.h"
#include "Arc.h"

#ifdef CLIPPER_NOT_KBOOL
#include "clipper.hpp"
using namespace clipper;
#endif

const Point operator*(const double &d, const Point &p){ return p * d;}
double Point::tolerance = 0.001;

double Point::length()const
{
    return sqrt( x*x + y*y );
}

void Point::normalize()
{
	double len = length();
	if(fabs(len)> 0.000000000000001)
		*this = (*this) / len;
}

CVertex::CVertex(int type, const Point& p, const Point& c, int user_data):m_type(type), m_p(p), m_c(c), m_user_data(user_data)
{
}

void CCurve::append(const CVertex& vertex)
{
	m_vertices.push_back(vertex);
}

bool CCurve::CheckForArc(const CVertex& prev_vt, std::list<const CVertex*>& might_be_an_arc, Arc &arc)
{
	// this examines the vertices in might_be_an_arc
	// if they do fit an arc, set arc to be the arc that they fit and return true
	// returns true, if arc added
	if(might_be_an_arc.size() < 2)return false;

	// find middle point
	int num = might_be_an_arc.size();
	int i = 0;
	const CVertex* mid_vt = NULL;
	int mid_i = (num-1)/2;
	for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++, i++)
	{
		if(i == mid_i)
		{
			mid_vt = *It;
			break;
		}
	}

	// create a circle to test
	Point p0(prev_vt.m_p);
	Point p1(mid_vt->m_p);
	Point p2(might_be_an_arc.back()->m_p);
	Circle c(p0, p1, p2);

	const double max_arc_radius = 1.0 / Point::tolerance;
	if (c.m_radius > max_arc_radius)
	{
		return(false);	// We don't want to produce an arc whose radius is too large.
	}

	const CVertex* current_vt = &prev_vt;
	double accuracy = CArea::m_accuracy * 1.4 / CArea::m_units;
	for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)
	{
		const CVertex* vt = *It;

		if(!c.LineIsOn(current_vt->m_p, vt->m_p, accuracy))
			return false;
		current_vt = vt;
	}

	arc.m_c = c.m_c;
	arc.m_s = prev_vt.m_p;
	arc.m_e = might_be_an_arc.back()->m_p;
	arc.SetDirWithPoint(might_be_an_arc.front()->m_p);
	arc.m_user_data = might_be_an_arc.back()->m_user_data;

	if(arc.IncludedAngle() >= 3.15)return false; // We don't want full arcs, so limit to about 180 degrees

	return true;
}

void CCurve::AddArcOrLines(bool check_for_arc, std::list<CVertex> &new_vertices, std::list<const CVertex*>& might_be_an_arc, Arc &arc, bool &arc_found, bool &arc_added)
{
	if(check_for_arc && CheckForArc(new_vertices.back(), might_be_an_arc, arc))
	{
		arc_found = true;
	}
	else
	{
		if(arc_found)
		{
			new_vertices.push_back(CVertex(arc.m_dir ? 1:-1, arc.m_e, arc.m_c, arc.m_user_data));
			arc_added = true;
			arc_found = false;
			const CVertex* back_vt = might_be_an_arc.back();
			might_be_an_arc.clear();
			if(check_for_arc)might_be_an_arc.push_back(back_vt);
		}
		else
		{
			const CVertex* back_vt = might_be_an_arc.back();
			if(check_for_arc)might_be_an_arc.pop_back();
			for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)
			{
				const CVertex* v = *It;
				new_vertices.push_back(*v);
			}
			might_be_an_arc.clear();
			if(check_for_arc)might_be_an_arc.push_back(back_vt);
		}
	}
}

void CCurve::FitArcs()
{
	std::list<CVertex> new_vertices;

	std::list<const CVertex*> might_be_an_arc;
	Arc arc;
	bool arc_found = false;
	bool arc_added = false;

	int i = 0;
	for(std::list<CVertex>::iterator It = m_vertices.begin(); It != m_vertices.end(); It++, i++)
	{
		CVertex& vt = *It;
		if(vt.m_type || i == 0)
			new_vertices.push_back(vt);
		else
		{
			might_be_an_arc.push_back(&vt);

			if(might_be_an_arc.size() == 1)
			{
			}
			else
			{
				AddArcOrLines(true, new_vertices, might_be_an_arc, arc, arc_found, arc_added);
			}
		}
	}

	if(might_be_an_arc.size() > 0)AddArcOrLines(false, new_vertices, might_be_an_arc, arc, arc_found, arc_added);

	if(arc_added)
	{
		m_vertices.clear();
		for(std::list<CVertex>::iterator It = new_vertices.begin(); It != new_vertices.end(); It++)m_vertices.push_back(*It);
		for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)m_vertices.push_back(*(*It));
	}
}

Point CCurve::NearestPoint(const Point& p)
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	bool best_point_valid = false;
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	for(std::list<CVertex>::iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		CVertex& vertex = *It;
		if(prev_p_valid)
		{
			Point near_point = SpanPtr(prev_p, vertex).NearestPoint(p);
			double dist = near_point.dist(p);
			if(!best_point_valid || dist < best_dist)
			{
				best_dist = dist;
				best_point = near_point;
				best_point_valid = true;
			}
		}
		prev_p = vertex.m_p;
		prev_p_valid = true;
	}
	return best_point;
}

void CCurve::GetBox(CBox &box)
{
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	for(std::list<CVertex>::iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		CVertex& vertex = *It;
		if(prev_p_valid)
		{
			SpanPtr(prev_p, vertex).GetBox(box);
		}
		prev_p = vertex.m_p;
		prev_p_valid = true;
	}
}

void CCurve::Reverse()
{
	std::list<CVertex> new_vertices;

	CVertex* prev_v = NULL;

	for(std::list<CVertex>::reverse_iterator It = m_vertices.rbegin(); It != m_vertices.rend(); It++)
	{
		CVertex &v = *It;
		int type = 0;
		Point cp(0.0, 0.0);
		if(prev_v)
		{
			type = -prev_v->m_type;
			cp = prev_v->m_c;
		}
		CVertex new_v(type, v.m_p, cp);
		new_vertices.push_back(new_v);
		prev_v = &v;
	}

	m_vertices = new_vertices;
}

Point SpanPtr::NearestPoint(const Point& p)
{
	if(m_v.m_type == 0)
	{
		Point Vs = (m_v.m_p - m_p);
		Vs.normalize();
		double dp = (p - m_p) * Vs;		
		return (Vs * dp) + m_p;
	}
	else
	{
		double radius = m_p.dist(m_v.m_c);
		double r = p.dist(m_v.m_c);
		if(r < Point::tolerance)return m_p;
		Point vc = (m_v.m_c - p);
		vc.normalize();
		Point pn = m_p + vc * ((r - radius) / r);
		return pn;
	}
}

static int GetQuadrant(const Point& v){
	// 0 = [+,+], 1 = [-,+], 2 = [-,-], 3 = [+,-]
	if(v.x > 0)
	{
		if(v.y > 0)
			return 0;
		return 3;
	}
	if(v.y > 0)
		return 1;
	return 2;
}

static Point QuadrantEndPoint(int i)
{
	if(i >3)i-=4;
	switch(i)
	{
	case 0:
		return Point(0.0,1.0);
	case 1:
		return Point(-1.0,0.0);
	case 2:
		return Point(0.0,-1.0);
	default:
		return Point(1.0,0.0);
	}
}

void SpanPtr::GetBox(CBox &box)
{
	box.Insert(m_p);
	box.Insert(m_v.m_p);

	if(this->m_v.m_type)
	{
		// arc, add quadrant points
		Point vs = m_p - m_v.m_c;
		Point ve = m_v.m_p - m_v.m_c;
		int qs = GetQuadrant(vs);
		int qe = GetQuadrant(ve);
		if(m_v.m_type == -1)
		{
			// swap qs and qe
			int t=qs;
			qs = qe;
			qe = t;
		}

		if(qe<qs)qe = qe + 4;

		double rad = m_v.m_p.dist(m_v.m_c);

		for(int i = qs; i<qe; i++)
		{
			box.Insert(m_v.m_c + QuadrantEndPoint(i) * rad);
		}
	}
}

double CArea::m_round_corners_factor = 1.5;
double CArea::m_accuracy = 0.01;
double CArea::m_units = 1.0;

#ifdef CLIPPER_NOT_KBOOL
#else
void CArea::ArmBoolEng( Bool_Engine* booleng )
{
    // set some global vals to arm the boolean engine
    double DGRID = 1000;  // round coordinate X or Y value in calculations to this
    double MARGE = 0.001;   // snap with in this range points to lines in the intersection routines
                          // should always be > DGRID  a  MARGE >= 10*DGRID is oke
                          // this is also used to remove small segments and to decide when
                          // two segments are in line.
    double CORRECTIONFACTOR = 500.0;  // correct the polygons by this number
	double CORRECTIONABER   = CArea::m_accuracy;    // the accuracy for the rounded shapes used in correction
    double ROUNDFACTOR      = 1.5;    // when will we round the correction shape to a circle
    double SMOOTHABER       = 10.0;   // accuracy when smoothing a polygon
    double MAXLINEMERGE     = 1000.0; // leave as is, segments of this length in smoothen
 

    // DGRID is only meant to make fractional parts of input data which 
    // are doubles, part of the integers used in vertexes within the boolean algorithm.
    // Within the algorithm all input data is multiplied with DGRID
    
    // space for extra intersection inside the boolean algorithms
    // only change this if there are problems
    int GRID =10000;

    booleng->SetMarge( MARGE );
    booleng->SetGrid( GRID );
    booleng->SetDGrid( DGRID );
    booleng->SetCorrectionFactor( CORRECTIONFACTOR );
    booleng->SetCorrectionAber( CORRECTIONABER );
    booleng->SetSmoothAber( SMOOTHABER );
    booleng->SetMaxlinemerge( MAXLINEMERGE );
    booleng->SetRoundfactor( ROUNDFACTOR );
}
#endif

void CArea::append(const CCurve& curve)
{
	m_curves.push_back(curve);
}

void CArea::Subtract(const CArea& a2)
{
#ifdef CLIPPER_NOT_KBOOL
	Clipper c;
	TPolyPolygon pp1, pp2;
	MakePolyPoly(pp1);
	a2.MakePolyPoly(pp2);
	c.AddPolyPolygon(pp1, ptSubject);
	c.AddPolyPolygon(pp2, ptClip);
	TPolyPolygon solution;
	c.Execute(ctDifference, solution);
	SetFromResult(solution);
#else
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( booleng, true );
	a2.MakeGroup( booleng, false );
	booleng->Do_Operation(BOOL_A_SUB_B);
	SetFromResult( booleng );
#endif
}

void CArea::Intersect(const CArea& a2)
{
#ifdef CLIPPER_NOT_KBOOL
	Clipper c;
	TPolyPolygon pp1, pp2;
	MakePolyPoly(pp1);
	a2.MakePolyPoly(pp2);
	c.AddPolyPolygon(pp1, ptSubject);
	c.AddPolyPolygon(pp2, ptClip);
	TPolyPolygon solution;
	c.Execute(ctIntersection, solution);
	SetFromResult(solution);
#else
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( booleng, true );
	a2.MakeGroup( booleng, false );
	booleng->Do_Operation(BOOL_AND);
	SetFromResult( booleng );
#endif
}

void CArea::Union(const CArea& a2)
{
#ifdef CLIPPER_NOT_KBOOL
	Clipper c;
	TPolyPolygon pp1, pp2;
	MakePolyPoly(pp1);
	a2.MakePolyPoly(pp2);
	c.AddPolyPolygon(pp1, ptSubject);
	c.AddPolyPolygon(pp2, ptClip);
	TPolyPolygon solution;
	c.Execute(ctUnion, solution);
	SetFromResult(solution);
#else
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( booleng, true );
	a2.MakeGroup( booleng, false );
	booleng->Do_Operation(BOOL_OR);
	SetFromResult( booleng );
#endif
}

void CArea::Offset(double inwards_value)
{
#ifdef CLIPPER_NOT_KBOOL
	TPolyPolygon pp, pp2;
	MakePolyPoly(pp);
	MakeObrounds(pp, pp2, -inwards_value);
	SetFromResult(pp2);
#else
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( booleng, true);
	booleng->SetRoundfactor(m_round_corners_factor);
	booleng->SetCorrectionFactor( -inwards_value * m_units );
	booleng->Do_Operation(BOOL_CORRECTION);
	SetFromResult( booleng );
#endif
}

#ifdef CLIPPER_NOT_KBOOL
void CArea::AddVertex(std::list<TDoublePoint> &pts, const CVertex& vertex, const CVertex* prev_vertex)const
{
	if(vertex.m_type == 0 || prev_vertex == NULL)
	{
		pts.push_back(DoublePoint(vertex.m_p.x * m_units, vertex.m_p.y * m_units));
	}
	else
	{
		double phi,dphi,dx,dy;
		int Segments;
		int i;
		double ang1,ang2,phit;

		dx = (prev_vertex->m_p.x - vertex.m_c.x) * m_units;
		dy = (prev_vertex->m_p.y - vertex.m_c.y) * m_units;

		ang1=atan2(dy,dx);
		if (ang1<0) ang1+=2.0*M_PI;
		dx = (vertex.m_p.x - vertex.m_c.x) * m_units;
		dy = (vertex.m_p.y - vertex.m_c.y) * m_units;
		ang2=atan2(dy,dx);
		if (ang2<0) ang2+=2.0*M_PI;

		if (vertex.m_type == -1)
		{ //clockwise
			if (ang2 > ang1)
				phit=2.0*M_PI-ang2+ ang1;
			else
				phit=ang1-ang2;
		}
		else
		{ //counter_clockwise
			if (ang1 > ang2)
				phit=-(2.0*M_PI-ang1+ ang2);
			else
				phit=-(ang2-ang1);
		}

		//what is the delta phi to get an accurancy of aber
		double radius = sqrt(dx*dx + dy*dy);
		dphi=2*acos((radius-m_accuracy)/radius);

		//set the number of segments
		if (phit > 0)
			Segments=(int)ceil(phit/dphi);
		else
			Segments=(int)ceil(-phit/dphi);

		if (Segments <= 1)
			Segments=1;
		if (Segments > 100)
			Segments=100;

		dphi=phit/(Segments);

		double px = prev_vertex->m_p.x * m_units;
		double py = prev_vertex->m_p.y * m_units;

		for (i=1; i<=Segments; i++)
		{
			dx = px - vertex.m_c.x * m_units;
			dy = py - vertex.m_c.y * m_units;
			phi=atan2(dy,dx);

			double nx = vertex.m_c.x * m_units + radius * cos(phi-dphi);
			double ny = vertex.m_c.y * m_units + radius * sin(phi-dphi);

			pts.push_back(DoublePoint(nx, ny));

			px = nx;
			py = ny;
		}
	}
}

bool IsPolygonClockwise(const TPolygon& p)
{
	double area = 0.0;
	unsigned int s = p.size();
	for(unsigned int i = 0; i<s; i++)
	{
		int im1 = i-1;
		if(im1 < 0)im1 += s;

		const TDoublePoint &pt0 = p[im1];
		const TDoublePoint &pt1 = p[i];

		area += 0.5 * (pt1.X - pt0.X) * (pt0.Y + pt1.Y);
	}

	return area > 0.0;
}

void CArea::MakeObrounds(const TPolyPolygon &pp, TPolyPolygon &pp_new, double radius)const
{
	Clipper c;

	printf("radius = %lf\n", radius);

	bool reverse = (radius < 0);

	for(unsigned int i = 0; i < pp.size(); i++)
	{
		const TPolygon& p = pp[i];
		bool clockwise = IsPolygonClockwise(p);
		//printf("polygon clockwise = %s\n", clockwise ? "CW" : "ACW");

		std::list<TDoublePoint> pts;

		if(p.size() > 2)
		{
			if(reverse)
			{
				for(unsigned int j = p.size()-1; j > 1; j--)MakeObround(p[j], p[j-1], p[j-2], pts, radius);
				MakeObround(p[1], p[0], p[p.size()-1], pts, radius);
				MakeObround(p[0], p[p.size()-1], p[p.size()-2], pts, radius);
			}
			else
			{
				MakeObround(p[p.size()-2], p[p.size()-1], p[0], pts, radius);
				MakeObround(p[p.size()-1], p[0], p[1], pts, radius);
				for(unsigned int j = 2; j < p.size(); j++)MakeObround(p[j-2], p[j-1], p[j], pts, radius);
			}
		}

		TPolygon loopy_polygon;
		loopy_polygon.reserve(pts.size());
		for(std::list<TDoublePoint>::iterator It = pts.begin(); It != pts.end(); It++)
		{
			loopy_polygon.push_back(*It);
		}
		c.AddPolygon(loopy_polygon, ptSubject);
	}

	//typedef enum { ctIntersection, ctUnion, ctDifference, ctXor } TClipType;
	c.ForceOrientation(false);
	c.Execute(ctXor, pp_new, pftNonZero, pftNonZero);
}

void CArea::MakeObround(const TDoublePoint &pt0, const TDoublePoint &pt1, const TDoublePoint &pt2, std::list<TDoublePoint> &pts, double radius)const
{
	Point p0(pt0.X, pt0.Y);
	Point p1(pt1.X, pt1.Y);
	Point p2(pt2.X, pt2.Y);
	Point forward0 = p1 - p0;
	Point right0(forward0.y, -forward0.x);
	right0.normalize();
	Point forward1 = p2 - p1;
	Point right1(forward1.y, -forward1.x);
	right1.normalize();

	int arc_dir = (radius > 0) ? 1 : -1;

	CVertex v0(0, p1 + right0 * radius, Point(0, 0));
	CVertex v1(arc_dir, p1 + right1 * radius, p1);
	CVertex v2(0, p1 + right1 * radius, Point(0, 0));

	AddVertex(pts, v1, &v0);
	AddVertex(pts, v2, &v1);
}

void CArea::MakePolyPoly( TPolyPolygon &pp )const{
	pp.clear();

	for(std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		std::list<TDoublePoint> pts;
		const CCurve& curve = *It;
		const CVertex* prev_vertex = NULL;
		for(std::list<CVertex>::const_iterator It2 = curve.m_vertices.begin(); It2 != curve.m_vertices.end(); It2++)
		{
			const CVertex& vertex = *It2;
			if(prev_vertex)AddVertex(pts, vertex, prev_vertex);
			prev_vertex = &vertex;
		}

		TPolygon p;
		p.reserve(pts.size());
		for(std::list<TDoublePoint>::iterator It = pts.begin(); It != pts.end(); It++)
		{
			p.push_back(*It);
		}
		pp.push_back(p);
	}
}

void CArea::SetFromResult( const TPolyPolygon& pp, resultType result_type )
{
	// delete existing geometry
	m_curves.clear();

	for(unsigned int i = 0; i < pp.size(); i++)
	{
		const TPolygon& p = pp[i];

		if(result_type != rtAll && (IsPolygonClockwise(p) != (result_type == rtClockwise)))
			continue;

		m_curves.push_back(CCurve());
		CCurve &curve = m_curves.back();
		for(unsigned int j = 0; j < p.size(); j++)
		{
			const TDoublePoint &pt = p[j];
			CVertex vertex(0, Point(pt.X / m_units, pt.Y / m_units), Point(0.0, 0.0));
			curve.m_vertices.push_back(vertex);
        }
		curve.m_vertices.push_back(curve.m_vertices.front()); // make a copy of the first point at the end
		curve.FitArcs();
    }
}

#else
void CArea::AddVertex(Bool_Engine* booleng, const CVertex& vertex, const CVertex* prev_vertex)const
{
	if(vertex.m_type == 0 || prev_vertex == NULL)
	{
		booleng->AddPoint(vertex.m_p.x * m_units, vertex.m_p.y * m_units, vertex.m_user_data);
	}
	else
	{
		double phi,dphi,dx,dy;
		int Segments;
		int i;
		double ang1,ang2,phit;

		dx = (prev_vertex->m_p.x - vertex.m_c.x) * m_units;
		dy = (prev_vertex->m_p.y - vertex.m_c.y) * m_units;

		ang1=atan2(dy,dx);
		if (ang1<0) ang1+=2.0*M_PI;
		dx = (vertex.m_p.x - vertex.m_c.x) * m_units;
		dy = (vertex.m_p.y - vertex.m_c.y) * m_units;
		ang2=atan2(dy,dx);
		if (ang2<0) ang2+=2.0*M_PI;

		if (vertex.m_type == -1)
		{ //clockwise
			if (ang2 > ang1)
				phit=2.0*M_PI-ang2+ ang1;
			else
				phit=ang1-ang2;
		}
		else
		{ //counter_clockwise
			if (ang1 > ang2)
				phit=-(2.0*M_PI-ang1+ ang2);
			else
				phit=-(ang2-ang1);
		}

		//what is the delta phi to get an accurancy of aber
		double radius = sqrt(dx*dx + dy*dy);
		dphi=2*acos((radius-booleng->GetCorrectionAber())/radius);

		//set the number of segments
		if (phit > 0)
			Segments=(int)ceil(phit/dphi);
		else
			Segments=(int)ceil(-phit/dphi);

		if (Segments <= 1)
			Segments=1;
		if (Segments > 100)
			Segments=100;

		dphi=phit/(Segments);

		double px = prev_vertex->m_p.x * m_units;
		double py = prev_vertex->m_p.y * m_units;

		for (i=1; i<=Segments; i++)
		{
			dx = px - vertex.m_c.x * m_units;
			dy = py - vertex.m_c.y * m_units;
			phi=atan2(dy,dx);

			double nx = vertex.m_c.x * m_units + radius * cos(phi-dphi);
			double ny = vertex.m_c.y * m_units + radius * sin(phi-dphi);

			booleng->AddPoint(nx, ny, vertex.m_user_data);

			px = nx;
			py = ny;
		}
	}
}

void CArea::MakeGroup( Bool_Engine* booleng, bool a_not_b )const
{
	booleng->SetLinkHoles(true);

		booleng->StartPolygonAdd(a_not_b ? GROUP_A:GROUP_B);
		bool first_curve = true;
		const CVertex* last_vertex = NULL;

	for(std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		const CVertex* prev_vertex = NULL;
		for(std::list<CVertex>::const_iterator It2 = curve.m_vertices.begin(); It2 != curve.m_vertices.end(); It2++)
		{
			const CVertex& vertex = *It2;
			AddVertex(booleng, vertex, prev_vertex);
			prev_vertex = &vertex;
			if(first_curve)last_vertex = &vertex;
		}

				if(!first_curve)
				{
				booleng->AddPoint(last_vertex->m_p.x * m_units, last_vertex->m_p.y * m_units, 0);
			}

				first_curve = false;
	}
		booleng->EndPolygonAdd();
	
}

void CArea::SetFromResult( Bool_Engine* booleng )
{
	// delete existing geometry
	m_curves.clear();

	while ( booleng->StartPolygonGet() )
    {
		m_curves.push_back(CCurve());
		CCurve &curve = m_curves.back();

        // foreach point in the polygon
        while ( booleng->PolygonHasMorePoints() )
        {
			CVertex vertex(0, Point(booleng->GetPolygonXPoint() / m_units, booleng->GetPolygonYPoint() / m_units), Point(0.0, 0.0), booleng->GetPolygonPointUserData());

			curve.m_vertices.push_back(vertex);
        }
		curve.m_vertices.push_back(curve.m_vertices.front()); // make a copy of the first point at the end

		curve.FitArcs();
        booleng->EndPolygonGet();
    }
}
#endif

void CArea::FitArcs(){
	for(std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		curve.FitArcs();
	}
}

Point CArea::NearestPoint(const Point& p)
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	for(std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		Point near_point = curve.NearestPoint(p);
		double dist = near_point.dist(p);
		if(It == m_curves.begin() || dist < best_dist)
		{
			best_dist = dist;
			best_point = near_point;
		}
	}
	return best_point;
}

void CArea::GetBox(CBox &box)
{
	for(std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		curve.GetBox(box);
	}
}