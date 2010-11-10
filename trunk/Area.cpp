// Area.cpp

// written by Dan Heeks, February 6th 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#include "Area.h"
#include "Circle.h"
#include "Arc.h"
#include "AreaOrderer.h"

const Point operator*(const double &d, const Point &p){ return p * d;}
double Point::tolerance = 0.001;

static const double PI = 3.1415926535897932;

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

CVertex::CVertex(const Point& p, int user_data):m_type(0), m_p(p), m_c(0.0, 0.0), m_user_data(user_data)
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
				if(It != might_be_an_arc.begin() || (new_vertices.size() == 0) || (new_vertices.back().m_p != v->m_p))
				{
					new_vertices.push_back(*v);
				}
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

Point CCurve::NearestPoint(const Point& p)const
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	bool best_point_valid = false;
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	bool first_span = true;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p_valid)
		{
			Point near_point = SpanPtr(prev_p, vertex, first_span).NearestPoint(p);
			first_span = false;
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

Point CCurve::NearestPoint(const CCurve& c, double *d)const
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	bool best_point_valid = false;
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	bool first_span = true;
	for(std::list<CVertex>::const_iterator It = c.m_vertices.begin(); It != c.m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p_valid)
		{
			double dist;
			Point near_point = NearestPoint(SpanPtr(prev_p, vertex, first_span), &dist);
			first_span = false;
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
	if(d)*d = best_dist;
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

double CCurve::GetArea()const
{
	double area = 0.0;
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p_valid)
		{
			area += SpanPtr(prev_p, vertex).GetArea();
		}
		prev_p = vertex.m_p;
		prev_p_valid = true;
	}
	return area;
}

void CCurve::ChangeStart(const Point &p) {
	CCurve new_curve;

	bool started = false;
	bool finished = false;
	int start_span;

	for(int i = 0; i < 2; i++)
	{
		const Point *prev_p = NULL;

		int span_index = 0;
		for(std::list<CVertex>::const_iterator VIt = m_vertices.begin(); VIt != m_vertices.end() && !finished; VIt++)
		{
			const CVertex& vertex = *VIt;

			if(prev_p)
			{
				SpanPtr span(*prev_p, vertex);
				if(span.On(p))
				{
					if(started)
					{
						if(p == *prev_p || span_index != start_span)
						{
							new_curve.m_vertices.push_back(vertex);
						}
						else
						{
							if(p == vertex.m_p)new_curve.m_vertices.push_back(vertex);
							else
							{
								CVertex v(vertex);
								v.m_p = p;
								new_curve.m_vertices.push_back(v);
							}
							finished = true;
						}
					}
					else
					{
						new_curve.m_vertices.push_back(CVertex(p));
						started = true;
						start_span = span_index;
						if(p != vertex.m_p)new_curve.m_vertices.push_back(vertex);
					}
				}
				else
				{
					if(started)
					{
						new_curve.m_vertices.push_back(vertex);
					}
				}
				span_index++;
			}
			prev_p = &(vertex.m_p);
		}
	}

	if(finished)
	{
		*this = new_curve;
	}
}

Point SpanPtr::NearestPointNotOnSpan(const Point& p)const
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
		return p + vc * ((r - radius) / r);
	}
}

Point SpanPtr::NearestPoint(const Point& p)const
{
	Point np = NearestPointNotOnSpan(p);
	double t = Parameter(np);
	if(t >= 0.0 && t <= 1.0)return np;

	double d1 = p.dist(this->m_p);
	double d2 = p.dist(this->m_v.m_p);

	if(d1 < d2)return this->m_p;
	else return m_v.m_p;
}

Point CCurve::NearestPoint(const SpanPtr& p, double *d)const
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	bool best_point_valid = false;
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	bool first_span = true;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p_valid)
		{
			double dist;
			Point near_point = SpanPtr(prev_p, vertex, first_span).NearestPoint(p, &dist);
			first_span = false;
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
	if(d)*d = best_dist;
	return best_point;
}

Point SpanPtr::MidPerim(double d)const {
	/// returns a point which is 0-d along span
	Point p;
	if(m_v.m_type == 0) {
		Point vs = m_v.m_p - m_p;

		p = vs * d + m_p;
	}
	else {
		Point v = m_p - m_v.m_c;
		double radius = m_p.dist(m_v.m_c);
		v.Rotate(d * m_v.m_type / radius);
		p = v + m_v.m_c;
	}
	return p;
}

Point SpanPtr::MidParam(double param)const {
	/// returns a point which is 0-1 along span
	if(fabs(param) < 0.00000000000001)return m_p;
	if(fabs(param - 1.0) < 0.00000000000001)return m_v.m_p;

	Point p;
	if(m_v.m_type == 0) {
		Point vs = m_v.m_p - m_p;
		p = vs * param + m_p;
	}
	else {
		Point v = m_p - m_v.m_c;
		double radius = m_p.dist(m_v.m_c);
		v.Rotate(param * IncludedAngle());
		p = v + m_v.m_c;
	}
	return p;
}

Point SpanPtr::NearestPointToSpan(const SpanPtr& p, double &d)const
{
	Point midpoint = MidParam(0.5);
	Point np = p.NearestPoint(m_p);
	Point best_point = m_p;
	double dist = np.dist(m_p);
	if(p.m_start_span)dist -= (CArea::m_accuracy * 2); // give start of curve most priority
	Point npm = p.NearestPoint(midpoint);
	double dm = npm.dist(midpoint) - CArea::m_accuracy; // lie about midpoint distance to give midpoints priority
	if(dm < dist){dist = dm; best_point = midpoint;}
	Point np2 = p.NearestPoint(m_v.m_p);
	double dp2 = np2.dist(m_v.m_p);
	if(dp2 < dist){dist = dp2; best_point = m_v.m_p;}
	d = dist;
	return best_point;
}

Point SpanPtr::NearestPoint(const SpanPtr& p, double *d)const
{
	double best_dist;
	Point best_point = this->NearestPointToSpan(p, best_dist);

	// try the other way round too
	double best_dist2;
	Point best_point2 = p.NearestPointToSpan(*this, best_dist2);
	if(best_dist2 < best_dist)
	{
		best_point = NearestPoint(best_point2);
		best_dist = best_dist2;
	}

	if(d)*d = best_dist;
	return best_point;
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

double IncludedAngle(const Point& v0, const Point& v1, int dir) {
	// returns the absolute included angle between 2 vectors in the direction of dir ( 1=acw  -1=cw)
	double inc_ang = v0 * v1;
	if(inc_ang > 1. - 1.0e-10) return 0;
	if(inc_ang < -1. + 1.0e-10)
		inc_ang = PI;  
	else {									// dot product,   v1 . v2  =  cos ang
		if(inc_ang > 1.0) inc_ang = 1.0;
		inc_ang = acos(inc_ang);									// 0 to pi radians

		if(dir * (v0 ^ v1) < 0) inc_ang = 2 * PI - inc_ang ;		// cp
	}
	return dir * inc_ang;
}

double SpanPtr::IncludedAngle()const
{
	if(m_v.m_type)
	{
		Point vs = ~(m_p - m_v.m_c);
		Point ve = ~(m_v.m_p - m_v.m_c);
		if(m_v.m_type == -1)
		{
			vs = -vs;
			ve = -ve;
		}
		vs.normalize();
		ve.normalize();

		return ::IncludedAngle(vs, ve, m_v.m_type);
	}

	return 0.0;
}

double SpanPtr::GetArea()const
{
	if(m_v.m_type)
	{
		double angle = IncludedAngle();
		double radius = m_p.dist(m_v.m_c);
		return ( 0.5 * ((m_v.m_c.x - m_p.x) * (m_v.m_c.y + m_p.y) - (m_v.m_c.x - m_v.m_p.x) * (m_v.m_c.y + m_v.m_p.y) - angle * radius * radius));
	}

	return 0.5 * (m_v.m_p.x - m_p.x) * (m_p.y + m_v.m_p.y);
}

double SpanPtr::Parameter(const Point& p)const
{
	double t;
	if(m_v.m_type == 0) {
		Point v0 = p - m_p;
		Point vs = m_v.m_p - m_p;
		double length = vs.length();
		vs.normalize();
		t = vs * v0;
		t = t / length;
	}
	else
	{
		// true if p lies on arc span sp (p must be on circle of span)
		Point vs = ~(m_p - m_v.m_c);
		Point v = ~(p - m_v.m_c);
		vs.normalize();
		v.normalize();
		if(m_v.m_type == -1){
			vs = -vs;
			v = -v;
		}
		double ang = ::IncludedAngle(vs, v, m_v.m_type);
		double angle = IncludedAngle();
		t = ang / angle;
	}
	return t;
}

bool SpanPtr::On(const Point& p, double* t)const
{
	if(p != NearestPoint(p))return false;
	if(t)*t = Parameter(p);
	return true;
}


double CArea::m_accuracy = 0.01;
double CArea::m_units = 1.0;
bool CArea::m_fit_arcs = true;

void CArea::append(const CCurve& curve)
{
	m_curves.push_back(curve);
}

void CArea::FitArcs(){
	for(std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		curve.FitArcs();
	}
}

Point CArea::NearestPoint(const Point& p)const
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	for(std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		const CCurve& curve = *It;
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

void CArea::Reorder()
{
	// curves may have been added with wrong directions
	// test all kurves to see which one are outsides and which are insides and 
	// make sure outsides are anti-clockwise and insides are clockwise

	// returns 0, if the curves are OK
	// returns 1, if the curves are overlapping

	CAreaOrderer ao;
	for(std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		ao.Insert(&curve);
	}

	*this = ao.ResultArea();
}

class ZigZag
{
public:
	CCurve zig;
	CCurve zag;
	ZigZag(const CCurve& Zig, const CCurve& Zag):zig(Zig), zag(Zag){}
};

static double stepover_for_pocket = 0.0;
static std::list<ZigZag> zigzag_list_for_zigs;
static std::list<CCurve> curve_list_for_zigs;
static bool rightward_for_zigs = true;
static double sin_angle_for_zigs = 0.0;
static double cos_angle_for_zigs = 0.0;
static double sin_minus_angle_for_zigs = 0.0;
static double cos_minus_angle_for_zigs = 0.0;
static double one_over_units = 0.0;

static Point rotated_point(const Point &p)
{
	return Point(p.x * cos_angle_for_zigs - p.y * sin_angle_for_zigs, p.x * sin_angle_for_zigs + p.y * cos_angle_for_zigs);
}
    
static Point unrotated_point(const Point &p)
{
    return Point(p.x * cos_minus_angle_for_zigs - p.y * sin_minus_angle_for_zigs, p.x * sin_minus_angle_for_zigs + p.y * cos_minus_angle_for_zigs);
}

static CVertex rotated_vertex(const CVertex &v)
{
	if(v.m_type)
	{
		return CVertex(v.m_type, rotated_point(v.m_p), rotated_point(v.m_c));
	}
    return CVertex(v.m_type, rotated_point(v.m_p), Point(0, 0));
}

static CVertex unrotated_vertex(const CVertex &v)
{
	if(v.m_type)
	{
		return CVertex(v.m_type, unrotated_point(v.m_p), unrotated_point(v.m_c));
	}
	return CVertex(v.m_type, unrotated_point(v.m_p), Point(0, 0));
}

static void rotate_area(CArea &a)
{
	for(std::list<CCurve>::iterator It = a.m_curves.begin(); It != a.m_curves.end(); It++)
	{
		CCurve& curve = *It;
		for(std::list<CVertex>::iterator CIt = curve.m_vertices.begin(); CIt != curve.m_vertices.end(); CIt++)
		{
			CVertex& vt = *CIt;
			vt = rotated_vertex(vt);
		}
	}
}

void test_y_point(int i, const Point& p, Point& best_p, bool &found, int &best_index, double y, bool left_not_right)
{
	// only consider points at y
	if(fabs(p.y - y) < 0.002 * one_over_units)
	{
		if(found)
		{
			// equal high point
			if(left_not_right)
			{
				// use the furthest left point
				if(p.x < best_p.x)
				{
					best_p = p;
					best_index = i;
				}
			}
			else
			{
				// use the furthest right point
				if(p.x > best_p.x)
				{
					best_p = p;
					best_index = i;
				}
			}
		}
		else
		{
			best_p = p;
			best_index = i;
			found = true;
		}
	}
}

static void make_zig_curve(const CCurve& input_curve, double y0, double y)
{
	CCurve curve(input_curve);

	if(rightward_for_zigs)
	{
		if(curve.IsClockwise())
			curve.Reverse();
	}
	else
	{
		if(!curve.IsClockwise())
			curve.Reverse();
	}

    // find a high point to start looking from
	Point top_left;
	int top_left_index;
	bool top_left_found = false;
	Point top_right;
	int top_right_index;
	bool top_right_found = false;
	Point bottom_left;
	int bottom_left_index;
	bool bottom_left_found = false;

	int i =0;
	for(std::list<CVertex>::const_iterator VIt = curve.m_vertices.begin(); VIt != curve.m_vertices.end(); VIt++, i++)
	{
		const CVertex& vertex = *VIt;

		test_y_point(i, vertex.m_p, top_right, top_right_found, top_right_index, y, !rightward_for_zigs);
		test_y_point(i, vertex.m_p, top_left, top_left_found, top_left_index, y, rightward_for_zigs);
		test_y_point(i, vertex.m_p, bottom_left, bottom_left_found, bottom_left_index, y0, rightward_for_zigs);
	}

	int start_index;
	int end_index;
	int zag_end_index;

	if(bottom_left_found)start_index = bottom_left_index;
	else if(top_left_found)start_index = top_left_index;

	if(top_right_found)
	{
		end_index = top_right_index;
		zag_end_index = top_left_index;
	}
	else
	{
		end_index = bottom_left_index;
		zag_end_index =  bottom_left_index;
	}
	if(end_index <= start_index)end_index += (i-1);
	if(zag_end_index <= start_index)zag_end_index += (i-1);

    CCurve zig, zag;
    
    bool zig_started = false;
    bool zig_finished = false;
    bool zag_finished = false;
    
	int v_index = 0;
	for(int i = 0; i < 2; i++)
	{
		// process the curve twice because we don't know where it will start
		if(zag_finished)
			break;
		for(std::list<CVertex>::const_iterator VIt = curve.m_vertices.begin(); VIt != curve.m_vertices.end(); VIt++)
		{
			if(i == 1 && VIt == curve.m_vertices.begin())
			{
				continue;
			}

			const CVertex& vertex = *VIt;

			if(zig_finished)
			{
				zag.m_vertices.push_back(unrotated_vertex(vertex));
				if(v_index == zag_end_index)
				{
					zag_finished = true;
					break;
				}
			}
			else if(zig_started)
			{
				zig.m_vertices.push_back(unrotated_vertex(vertex));
				if(v_index == end_index)
				{
					zig_finished = true;
					if(v_index == zag_end_index)
					{
						zag_finished = true;
						break;
					}
					zag.m_vertices.push_back(unrotated_vertex(vertex));
				}
			}
			else
			{
				if(v_index == start_index)
				{
					zig.m_vertices.push_back(unrotated_vertex(vertex));
					zig_started = true;
				}
			}
			v_index++;
		}
	}
        
    if(zig_finished)
		zigzag_list_for_zigs.push_back(ZigZag(zig, zag));
}

void make_zig(const CArea &a, double y0, double y)
{
	for(std::list<CCurve>::const_iterator It = a.m_curves.begin(); It != a.m_curves.end(); It++)
	{
		const CCurve &curve = *It;
		make_zig_curve(curve, y0, y);
	}
}
        
std::list< std::list<ZigZag> > reorder_zig_list_list;
        
void add_reorder_zig(ZigZag &zigzag)
{
    // look in existing lists

	// see if the zag is part of an existing zig
	if(zigzag.zag.m_vertices.size() > 1)
	{
		const Point& zag_e = zigzag.zag.m_vertices.front().m_p;
		bool zag_removed = false;
		for(std::list< std::list<ZigZag> >::iterator It = reorder_zig_list_list.begin(); It != reorder_zig_list_list.end() && !zag_removed; It++)
		{
			std::list<ZigZag> &zigzag_list = *It;
			for(std::list<ZigZag>::iterator It2 = zigzag_list.begin(); It2 != zigzag_list.end() && !zag_removed; It2++)
			{
				const ZigZag& z = *It2;
				for(std::list<CVertex>::const_iterator It3 = z.zig.m_vertices.begin(); It3 != z.zig.m_vertices.end() && !zag_removed; It3++)
				{
					const CVertex &v = *It3;
					if((fabs(zag_e.x - v.m_p.x) < (0.002 * one_over_units)) && (fabs(zag_e.y - v.m_p.y) < (0.002 * one_over_units)))
					{
						// remove zag from zigzag
						zigzag.zag.m_vertices.clear();
						zag_removed = true;
					}
				}
			}
		}
	}

	// see if the zigzag can join the end of an existing list
	const Point& zig_s = zigzag.zig.m_vertices.front().m_p;
	for(std::list< std::list<ZigZag> >::iterator It = reorder_zig_list_list.begin(); It != reorder_zig_list_list.end(); It++)
	{
		std::list<ZigZag> &zigzag_list = *It;
		const ZigZag& last_zigzag = zigzag_list.back();
        const Point& e = last_zigzag.zig.m_vertices.back().m_p;
        if((fabs(zig_s.x - e.x) < (0.002 * one_over_units)) && (fabs(zig_s.y - e.y) < (0.002 * one_over_units)))
		{
            zigzag_list.push_back(zigzag);
			return;
		}
	}
        
    // else add a new list
    std::list<ZigZag> zigzag_list;
    zigzag_list.push_back(zigzag);
    reorder_zig_list_list.push_back(zigzag_list);
}

void reorder_zigs()
{
	for(std::list<ZigZag>::iterator It = zigzag_list_for_zigs.begin(); It != zigzag_list_for_zigs.end(); It++)
	{
		ZigZag &zigzag = *It;
        add_reorder_zig(zigzag);
	}
        
	zigzag_list_for_zigs.clear();

	for(std::list< std::list<ZigZag> >::iterator It = reorder_zig_list_list.begin(); It != reorder_zig_list_list.end(); It++)
	{
		std::list<ZigZag> &zigzag_list = *It;
		if(zigzag_list.size() == 0)continue;

		curve_list_for_zigs.push_back(CCurve());
		for(std::list<ZigZag>::const_iterator It = zigzag_list.begin(); It != zigzag_list.end();)
		{
			const ZigZag &zigzag = *It;
			for(std::list<CVertex>::const_iterator It2 = zigzag.zig.m_vertices.begin(); It2 != zigzag.zig.m_vertices.end(); It2++)
			{
				if(It2 == zigzag.zig.m_vertices.begin() && It != zigzag_list.begin())continue; // only add the first vertex if doing the first zig
				const CVertex &v = *It2;
				curve_list_for_zigs.back().m_vertices.push_back(v);
			}

			It++;
			if(It == zigzag_list.end())
			{
				for(std::list<CVertex>::const_iterator It2 = zigzag.zag.m_vertices.begin(); It2 != zigzag.zag.m_vertices.end(); It2++)
				{
					if(It2 == zigzag.zag.m_vertices.begin())continue; // don't add the first vertex of the zag
					const CVertex &v = *It2;
					curve_list_for_zigs.back().m_vertices.push_back(v);
				}
			}
		}
	}
	reorder_zig_list_list.clear();
}

static void zigzag(const CArea &input_a)
{
	if(input_a.m_curves.size() == 0)
		return;
    
    one_over_units = 1 / CArea::m_units;
    
	CArea a(input_a);
    rotate_area(a);
    
    CBox b;
	a.GetBox(b);
    
    double x0 = b.MinX() - 1.0;
    double x1 = b.MaxX() + 1.0;

    double height = b.MaxY() - b.MinY();
    int num_steps = int(height / stepover_for_pocket + 1);
    double y = b.MinY();// + 0.1 * one_over_units;
    Point null_point(0, 0);
    rightward_for_zigs = true;
	curve_list_for_zigs.clear();
    
    for(int i = 0; i<num_steps; i++)
	{
        double y0 = y;
        y = y + stepover_for_pocket;
        Point p0(x0, y0);
        Point p1(x0, y);
        Point p2(x1, y);
        Point p3(x1, y0);
        CCurve c;
        c.m_vertices.push_back(CVertex(0, p0, null_point, 0));
        c.m_vertices.push_back(CVertex(0, p1, null_point, 0));
        c.m_vertices.push_back(CVertex(0, p2, null_point, 1));
        c.m_vertices.push_back(CVertex(0, p3, null_point, 0));
        c.m_vertices.push_back(CVertex(0, p0, null_point, 1));
        CArea a2;
		a2.m_curves.push_back(c);
        a2.Intersect(a);
        make_zig(a2, y0, y);
        rightward_for_zigs = !rightward_for_zigs;
	}
        
    reorder_zigs();
}

void CArea::MakePocketToolpath(std::list<CCurve> &curve_list, const CAreaPocketParams &params)const
{
	double radians_angle = params.zig_angle * PI / 180;
	sin_angle_for_zigs = sin(-radians_angle);
	cos_angle_for_zigs = cos(-radians_angle);
	sin_minus_angle_for_zigs = sin(radians_angle);
	cos_minus_angle_for_zigs = cos(radians_angle);
	stepover_for_pocket = params.stepover;

	CArea a_offset = *this;
	double current_offset = params.tool_radius + params.extra_offset;
	a_offset.Offset(current_offset);
        
    if(params.use_zig_zag)
	{
		zigzag(a_offset);
		curve_list = curve_list_for_zigs;
	}
	else
	{
		std::list<CArea> m_areas;
		a_offset.Split(m_areas);

		for(std::list<CArea>::iterator It = m_areas.begin(); It != m_areas.end(); It++)
		{
			CArea &a2 = *It;

			curve_list.push_back(CCurve());
			a2.MakeOnePocketCurve(curve_list.back(), params);
		}
	}
}

void CArea::Split(std::list<CArea> &m_areas)
{
	if(HolesLinked())
	{
		for(std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
		{
			CCurve& curve = *It;
			m_areas.push_back(CArea());
			m_areas.back().m_curves.push_back(curve);
		}
	}
	else
	{
		Reorder();
		for(std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
		{
			CCurve& curve = *It;
			if(curve.IsClockwise())
			{
				if(m_areas.size() > 0)
					m_areas.back().m_curves.push_back(curve);
			}
			else
			{
				m_areas.push_back(CArea());
				m_areas.back().m_curves.push_back(curve);
			}
		}
	}
}

eOverlapType GetOverlapType(const CCurve& c1, const CCurve& c2)
{
	CArea a1;
	a1.m_curves.push_back(c1);
	CArea a2;
	a2.m_curves.push_back(c2);

	return GetOverlapType(a1, a2);
}

eOverlapType GetOverlapType(const CArea& a1, const CArea& a2)
{
	CArea A1(a1);

	A1.Subtract(a2);
	if(A1.m_curves.size() == 0)
	{
		return eInside;
	}

	CArea A2(a2);
	A2.Subtract(a1);
	if(A2.m_curves.size() == 0)
	{
		return eOutside;
	}

	A1 = a1;
	A1.Intersect(a2);
	if(A1.m_curves.size() == 0)
	{
		return eSiblings;
	}

	return eCrossing;
}
