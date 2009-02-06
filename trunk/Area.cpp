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

void CArea::Offset(double inwards_value)
{
    Bool_Engine* booleng = new Bool_Engine();
    ArmBoolEng( booleng );
	MakeGroupA( booleng );
    booleng->SetCorrectionFactor( -inwards_value );
	booleng->Do_Operation(BOOL_CORRECTION);
	SetFromResult( booleng );
}

void CArea::MakeGroupA( Bool_Engine* booleng )
{
	booleng->StartPolygonAdd(GROUP_A);
	// to do, separate curves

	// for now, just assume one curve
	for(std::vector<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		for(std::vector<CVertex>::iterator It2 = curve.m_vertices.begin(); It2 != curve.m_vertices.end(); It2++)
		{
			CVertex& vertex = *It2;
			// to do, add arcs
			booleng->AddPoint(vertex.m_p[0], vertex.m_p[1]);
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

