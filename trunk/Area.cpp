// Area.cpp

// written by Dan Heeks, February 6th 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#include "Area.h"
#include "kbool/include/_lnk_itr.h"
#include "kbool/include/booleng.h"
#include "Circle.h"
#include "Arc.h"

CVertex::CVertex(int type, double x, double y, double cx, double cy):m_type(type)
{
	m_p[0] = x;
	m_p[1] = y;
	m_c[0] = cx;
	m_c[1] = cy;
}

bool CCurve::CheckForArc(const CVertex& prev_vt, std::list<const CVertex*>& might_be_an_arc, Arc &arc)
{
	// this examines the vertices in might_be_an_arc
	// if they do fit an arc, set arc to be the arc that they fit and return true
	// returns true, if arc added
	if(might_be_an_arc.size() < 2)return false;

	// create a circle to test
	Point p0(prev_vt.m_p[0], prev_vt.m_p[1]);
	Point p1(might_be_an_arc.front()->m_p[0], might_be_an_arc.front()->m_p[1]);
	Point p2(might_be_an_arc.back()->m_p[0], might_be_an_arc.back()->m_p[1]);
	Circle c(p0, p1, p2);

	const CVertex* current_vt = &prev_vt;
	for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)
	{
		const CVertex* vt = *It;

		if(!c.LineIsOn(current_vt->m_p, vt->m_p, CArea::m_accuracy * 1.4))
			return false;
		current_vt = vt;
	}

	arc.m_c = c.m_c;
	arc.m_s = prev_vt.m_p;
	arc.m_e = might_be_an_arc.back()->m_p;
	arc.SetDirWithPoint(might_be_an_arc.front()->m_p);

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
			new_vertices.push_back(CVertex(arc.m_dir ? 1:-1, arc.m_e.x, arc.m_e.y, arc.m_c.x, arc.m_c.y));
			arc_added = true;
			arc_found = false;
			const CVertex* back_vt = might_be_an_arc.back();
			might_be_an_arc.clear();
			might_be_an_arc.push_back(back_vt);
		}
		else
		{
			const CVertex* back_vt = might_be_an_arc.back();
			might_be_an_arc.pop_back();
			for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)
			{
				const CVertex* v = *It;
				new_vertices.push_back(*v);
			}
			might_be_an_arc.clear();
			might_be_an_arc.push_back(back_vt);
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

	int num = m_vertices.size();
	for(int i = 0; i<num; i++)
	{
		CVertex& vt = m_vertices[i];
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

	AddArcOrLines(false, new_vertices, might_be_an_arc, arc, arc_found, arc_added);

	if(arc_added)
	{
		m_vertices.clear();
		m_vertices.reserve(new_vertices.size() + might_be_an_arc.size());
		int i = 0;
		for(std::list<CVertex>::iterator It = new_vertices.begin(); It != new_vertices.end(); It++)m_vertices.push_back(*It);
		for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)m_vertices.push_back(*(*It));
	}
}

double CArea::m_round_corners_factor = 1.5;
double CArea::m_accuracy = 0.01;

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

void CArea::Subtract(const CArea& a2)
{
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( booleng, true );
//	a2.MakeGroup( booleng, false );
	//booleng->Do_Operation(BOOL_SUBTRACT);
	SetFromResult( booleng );
}

void CArea::Offset(double inwards_value)
{
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( booleng, true);
	booleng->SetRoundfactor(m_round_corners_factor);
	booleng->SetCorrectionFactor( -inwards_value );
	booleng->Do_Operation(BOOL_CORRECTION);
	SetFromResult( booleng );
}

void CArea::AddVertex(Bool_Engine* booleng, CVertex& vertex, CVertex* prev_vertex)
{
	if(vertex.m_type == 0 || prev_vertex == NULL)
	{
		booleng->AddPoint(vertex.m_p[0], vertex.m_p[1]);
	}
	else
	{
		double phi,dphi,dx,dy;
		int Segments;
		int i;
		double ang1,ang2,phit;

		dx = prev_vertex->m_p[0] - vertex.m_c[0];
		dy = prev_vertex->m_p[1] - vertex.m_c[1];

		ang1=atan2(dy,dx);
		if (ang1<0) ang1+=2.0*M_PI;
		dx = vertex.m_p[0] - vertex.m_c[0];
		dy = vertex.m_p[1] - vertex.m_c[1];
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

		double px = prev_vertex->m_p[0];
		double py = prev_vertex->m_p[1];

		for (i=1; i<=Segments; i++)
		{
			dx = px - vertex.m_c[0];
			dy = py - vertex.m_c[1];
			phi=atan2(dy,dx);

			double nx = vertex.m_c[0] + radius * cos(phi-dphi);
			double ny = vertex.m_c[1] + radius * sin(phi-dphi);

			booleng->AddPoint(nx, ny);

			px = nx;
			py = ny;
		}
	}
}

void CArea::MakeGroup( Bool_Engine* booleng, bool a_not_b )
{
	booleng->StartPolygonAdd(a_not_b ? GROUP_A:GROUP_B);
	// to do, separate curves

	// for now, just assume one curve
	for(std::vector<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		CVertex* prev_vertex = NULL;
		for(std::vector<CVertex>::iterator It2 = curve.m_vertices.begin(); It2 != curve.m_vertices.end(); It2++)
		{
			CVertex& vertex = *It2;
			AddVertex(booleng, vertex, prev_vertex);
			prev_vertex = &vertex;
		}
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
			CVertex vertex(0, booleng->GetPolygonXPoint(), booleng->GetPolygonYPoint(), 0.0, 0.0);

			curve.m_vertices.push_back(vertex);
        }
		curve.m_vertices.push_back(curve.m_vertices.front()); // make a copy of the first point at the end

		curve.FitArcs();
        booleng->EndPolygonGet();
    }
}

void CArea::FitArcs(){
	for(std::vector<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		curve.FitArcs();
	}
}