// Area.h

// written by Dan Heeks, February 6th 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

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
	void AddVertex(Bool_Engine* booleng, CVertex& vertex, CVertex* prev_vertex = NULL);

public:
	std::vector<CCurve> m_curves;
	static double m_round_corners_factor; // 1.0 for round 90 degree corners, 1.5 for square 90 degree corners

	void Offset(double inwards_value);
};

#endif // #define AREA_HEADER
