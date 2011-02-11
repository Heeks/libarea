// AreaBoolean.cpp

// implements CArea methods using Klaas Holwerda's Boolean
//    Licence: see kboollicense.txt 

#include "Area.h"
#include "kbool/include/_lnk_itr.h"
#include "kbool/include/booleng.h"

bool CArea::HolesLinked()const{ return true; }

static void ArmBoolEng( Bool_Engine* booleng )
{
    // set some global vals to arm the boolean engine
    double DGRID = 1000;  // round coordinate X or Y value in calculations to this
    double MARGE = 0.001;   // snap with in this range points to lines in the intersection routines
                          // should always be > DGRID  a  MARGE >= 10*DGRID is oke
                          // this is also used to remove small segments and to decide when
                          // two segments are in line.
    double CORRECTIONFACTOR = 500.0;  // correct the polygons by this number
	double CORRECTIONABER   = CArea::m_accuracy;    // the accuracy for the rounded shapes used in correction
    double ROUNDFACTOR      = 1.0;    // when will we round the correction shape to a circle
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

static void AddVertex(Bool_Engine* booleng, const CVertex& vertex, const CVertex* prev_vertex)
{
	if(vertex.m_type == 0 || prev_vertex == NULL)
	{
		booleng->AddPoint(vertex.m_p.x * CArea::m_units, vertex.m_p.y * CArea::m_units, vertex.m_user_data);
	}
	else
	{
		double phi,dphi,dx,dy;
		int Segments;
		int i;
		double ang1,ang2,phit;

		dx = (prev_vertex->m_p.x - vertex.m_c.x) * CArea::m_units;
		dy = (prev_vertex->m_p.y - vertex.m_c.y) * CArea::m_units;

		ang1=atan2(dy,dx);
		if (ang1<0) ang1+=2.0*M_PI;
		dx = (vertex.m_p.x - vertex.m_c.x) * CArea::m_units;
		dy = (vertex.m_p.y - vertex.m_c.y) * CArea::m_units;
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

		double px = prev_vertex->m_p.x * CArea::m_units;
		double py = prev_vertex->m_p.y * CArea::m_units;

		for (i=1; i<=Segments; i++)
		{
			dx = px - vertex.m_c.x * CArea::m_units;
			dy = py - vertex.m_c.y * CArea::m_units;
			phi=atan2(dy,dx);

			double nx = vertex.m_c.x * CArea::m_units + radius * cos(phi-dphi);
			double ny = vertex.m_c.y * CArea::m_units + radius * sin(phi-dphi);

			booleng->AddPoint(nx, ny, vertex.m_user_data);

			px = nx;
			py = ny;
		}
	}
}

static void MakeGroup( const CArea &area, Bool_Engine* booleng, bool a_not_b )
{
	booleng->SetLinkHoles(true);

		booleng->StartPolygonAdd(a_not_b ? GROUP_A:GROUP_B);
		bool first_curve = true;
		const CVertex* last_vertex = NULL;

	for(std::list<CCurve>::const_iterator It = area.m_curves.begin(); It != area.m_curves.end(); It++)
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
				booleng->AddPoint(last_vertex->m_p.x * CArea::m_units, last_vertex->m_p.y * CArea::m_units, 0);
			}

				first_curve = false;
	}
		booleng->EndPolygonAdd();
	
}

static void SetFromResult( CArea &area, Bool_Engine* booleng )
{
	// delete existing geometry
	area.m_curves.clear();

	while ( booleng->StartPolygonGet() )
    {
		area.m_curves.push_back(CCurve());
		CCurve &curve = area.m_curves.back();

        // foreach point in the polygon
        while ( booleng->PolygonHasMorePoints() )
        {
			CVertex vertex(0, Point(booleng->GetPolygonXPoint() / CArea::m_units, booleng->GetPolygonYPoint() / CArea::m_units), Point(0.0, 0.0), booleng->GetPolygonPointUserData());

			curve.m_vertices.push_back(vertex);
        }
		curve.m_vertices.push_back(curve.m_vertices.front()); // make a copy of the first point at the end

		curve.FitArcs();
        booleng->EndPolygonGet();
    }
}

void CArea::Subtract(const CArea& a2)
{
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( *this, booleng, true );
	MakeGroup( a2, booleng, false );
	booleng->Do_Operation(BOOL_A_SUB_B);
	SetFromResult( *this, booleng );
}

void CArea::Intersect(const CArea& a2)
{
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( *this, booleng, true );
	MakeGroup( a2, booleng, false );
	booleng->Do_Operation(BOOL_AND);
	SetFromResult( *this, booleng );
}

void CArea::Union(const CArea& a2)
{
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( *this, booleng, true );
	MakeGroup( a2, booleng, false );
	booleng->Do_Operation(BOOL_OR);
	SetFromResult( *this, booleng );
}

void CArea::Offset(double inwards_value)
{
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroup( *this, booleng, true);
	booleng->SetCorrectionFactor( -inwards_value * m_units );
	booleng->Do_Operation(BOOL_CORRECTION);
	SetFromResult( *this, booleng );
}
