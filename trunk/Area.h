// Area.h

#ifndef AREA_HEADER
#define AREA_HEADER

#include <vector>
#include <list>
#include <math.h>
class Bool_Engine;

class CVertex
{
public:
	int m_type; // 0 - line ( or start point ), 1 - anti-clockwise arc, -1 - clockwise arc
	double m_p[2]; // end point
	double m_c[2]; // centre point in absolute coordinates

	CVertex(int type, double x, double y, double cx, double cy);
};

class CCurve
{
	// a closed curve, please make sure you add an end point, the same as the start point
public:
	std::vector<CVertex> m_vertices;
};

class CArea
{
	void MakeGroupA( Bool_Engine* booleng );
	void SetFromResult( Bool_Engine* booleng );

public:
	std::vector<CCurve> m_curves;

	void Offset(double inwards_value);
};

#endif // #define AREA_HEADER