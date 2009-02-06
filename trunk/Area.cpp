// Area.cpp

#include "Area.h"
#include "kbool/include/_lnk_itr.h"
#include "kbool/include/booleng.h"

extern void ArmBoolEng( Bool_Engine* booleng );


CVertex::CVertex(int type, double x, double y, double cx, double cy):m_type(type)
{
	m_p[0] = x;
	m_p[1] = y;
	m_c[0] = cx;
	m_c[1] = cy;
}

double CArea::m_round_corners_factor = 1.5;

void CArea::Offset(double inwards_value)
{
	Bool_Engine* booleng = new Bool_Engine();
	ArmBoolEng( booleng );
	MakeGroupA( booleng );
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

void CArea::MakeGroupA( Bool_Engine* booleng )
{
	booleng->StartPolygonAdd(GROUP_A);
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
		CCurve curve;

        // foreach point in the polygon
        while ( booleng->PolygonHasMorePoints() )
        {
			CVertex vertex(0, booleng->GetPolygonXPoint(), booleng->GetPolygonYPoint(), 0.0, 0.0);

			curve.m_vertices.push_back(vertex);
        }
		curve.m_vertices.push_back(curve.m_vertices.front()); // make a copy of the first point at the end
        booleng->EndPolygonGet();
		m_curves.push_back(curve);
    }
}

