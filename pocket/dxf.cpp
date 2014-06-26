// dxf.cpp

// copyright Dan Heeks, September 3rd 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt
// copied from dxf.cpp in HeeksCAD

#include "stdafx.h"
#include "dxf.h"
#include "../Area.h"
#include <cmath>
#include <sstream>
#include <iostream>


static std::string str_for_Ttc;

const char* Ttc(const _TCHAR* str)
{
	// convert a wchar_t* string into a char* string
	str_for_Ttc.clear();
	while (*str)
		str_for_Ttc.push_back((char) *str++);
	return str_for_Ttc.c_str();
}

CDxfWrite::CDxfWrite(const _TCHAR* filepath)
{
	// start the file
	m_fail = false;
#ifdef __WXMSW__
	m_ofs = new ofstream(filepath, ios::out);
#else
	m_ofs = new ofstream(Ttc(filepath), ios::out);
#endif
	if(!(*m_ofs)){
		m_fail = true;
		return;
	}
	m_ofs->imbue(std::locale("C"));

	// start
	(*m_ofs) << 0          << endl;
	(*m_ofs) << "SECTION"  << endl;
	(*m_ofs) << 2          << endl;
	(*m_ofs) << "ENTITIES" << endl;
}

CDxfWrite::~CDxfWrite()
{
	// end
	(*m_ofs) << 0          << endl;
	(*m_ofs) << "ENDSEC"   << endl;
	(*m_ofs) << 0          << endl;
	(*m_ofs) << "EOF";

	delete m_ofs;
}

void CDxfWrite::WriteLine(const double* s, const double* e)
{
	(*m_ofs) << 0			<< endl;
	(*m_ofs) << "LINE"		<< endl;
	(*m_ofs) << 8			<< endl;	// Group code for layer name
	(*m_ofs) << 0			<< endl;	// Layer number
	(*m_ofs) << 10			<< endl;	// Start point of line
	(*m_ofs) << s[0]		<< endl;	// X in WCS coordinates
	(*m_ofs) << 20			<< endl;
	(*m_ofs) << s[1]		<< endl;	// Y in WCS coordinates
	(*m_ofs) << 30			<< endl;
	(*m_ofs) << s[2]		<< endl;	// Z in WCS coordinates
	(*m_ofs) << 11			<< endl;	// End point of line
	(*m_ofs) << e[0]		<< endl;	// X in WCS coordinates
	(*m_ofs) << 21			<< endl;
	(*m_ofs) << e[1]		<< endl;	// Y in WCS coordinates
	(*m_ofs) << 31			<< endl;
	(*m_ofs) << e[2]		<< endl;	// Z in WCS coordinates
}

void CDxfWrite::WriteArc(const double* s, const double* e, const double* c, bool dir)
{
	double ax = s[0] - c[0];
	double ay = s[1] - c[1];
	double bx = e[0] - c[0];
	double by = e[1] - c[1];

	double start_angle = atan2(ay, ax) * 180/Pi;
	double end_angle = atan2(by, bx) * 180/Pi;
	double radius = sqrt(ax*ax + ay*ay);
	if(!dir){
		double temp = start_angle;
		start_angle = end_angle;
		end_angle = temp;
	}
	(*m_ofs) << 0			<< endl;
	(*m_ofs) << "ARC"		<< endl;
	(*m_ofs) << 8			<< endl;	// Group code for layer name
	(*m_ofs) << 0			<< endl;	// Layer number
	(*m_ofs) << 10			<< endl;	// Centre X
	(*m_ofs) << c[0]		<< endl;	// X in WCS coordinates
	(*m_ofs) << 20			<< endl;
	(*m_ofs) << c[1]		<< endl;	// Y in WCS coordinates
	(*m_ofs) << 30			<< endl;
	(*m_ofs) << c[2]		<< endl;	// Z in WCS coordinates
	(*m_ofs) << 40			<< endl;	// 
	(*m_ofs) << radius		<< endl;	// Radius
	(*m_ofs) << 50			<< endl;
	(*m_ofs) << start_angle	<< endl;	// Start angle
	(*m_ofs) << 51			<< endl;
	(*m_ofs) << end_angle	<< endl;	// End angle
}

void CDxfWrite::WriteCircle(const double* c, double radius)
{
	(*m_ofs) << 0			<< endl;
	(*m_ofs) << "CIRCLE"		<< endl;
	(*m_ofs) << 8			<< endl;	// Group code for layer name
	(*m_ofs) << 0			<< endl;	// Layer number
	(*m_ofs) << 10			<< endl;	// Centre X
	(*m_ofs) << c[0]		<< endl;	// X in WCS coordinates
	(*m_ofs) << 20			<< endl;
	(*m_ofs) << c[1]		<< endl;	// Y in WCS coordinates
	(*m_ofs) << 30			<< endl;
	(*m_ofs) << c[2]		<< endl;	// Z in WCS coordinates
	(*m_ofs) << 40			<< endl;	// 
	(*m_ofs) << radius		<< endl;	// Radius
}

void CDxfWrite::WriteEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir)
{
	double m[3];
	m[2]=0;
	m[0] = major_radius * sin(rotation);
	m[1] = major_radius * cos(rotation);

	double ratio = minor_radius/major_radius;

	if(!dir){
		double temp = start_angle;
		start_angle = end_angle;
		end_angle = temp;
	}
	(*m_ofs) << 0			<< endl;
	(*m_ofs) << "ELLIPSE"		<< endl;
	(*m_ofs) << 8			<< endl;	// Group code for layer name
	(*m_ofs) << 0			<< endl;	// Layer number
	(*m_ofs) << 10			<< endl;	// Centre X
	(*m_ofs) << c[0]		<< endl;	// X in WCS coordinates
	(*m_ofs) << 20			<< endl;
	(*m_ofs) << c[1]		<< endl;	// Y in WCS coordinates
	(*m_ofs) << 30			<< endl;
	(*m_ofs) << c[2]		<< endl;	// Z in WCS coordinates
	(*m_ofs) << 40			<< endl;	// 
	(*m_ofs) << ratio		<< endl;	// Ratio
	(*m_ofs) << 11			<< endl;	// 
	(*m_ofs) << m[0]		<< endl;	// Major X 
	(*m_ofs) << 21			<< endl;
	(*m_ofs) << m[1]		<< endl;	// Major Y 
	(*m_ofs) << 31			<< endl;
	(*m_ofs) << m[2]		<< endl;	// Major Z 
	(*m_ofs) << 41		<< endl;
	(*m_ofs) << start_angle	<< endl;	// Start angle
	(*m_ofs) << 42		<< endl;
	(*m_ofs) << end_angle	<< endl;	// End angle
}

CDxfRead::CDxfRead(const _TCHAR* filepath)
{
	// start the file
	m_fail = false;
	m_ifs = new ifstream(Ttc(filepath));
	if(!(*m_ifs)){
		m_fail = true;
		return;
	}
	m_ifs->imbue(std::locale("C"));
}

CDxfRead::~CDxfRead()
{
	delete m_ifs;
}

bool CDxfRead::ReadLine(bool undoably)
{
	double s[3] = {0, 0, 0};
	double e[3] = {0, 0, 0};

	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf_s(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with line
				OnReadLine(s, e, undoably);
				return true;
			case 10:
				// start x
				get_line();
				ss.str(m_str); ss >> s[0]; if(ss.fail()) return false;
				break;
			case 20:
				// start y
				get_line();
				ss.str(m_str); ss >> s[1]; if(ss.fail()) return false;
				break;
			case 30:
				// start z
				get_line();
				ss.str(m_str); ss >> s[2]; if(ss.fail()) return false;
				break;
			case 11:
				// end x
				get_line();
				ss.str(m_str); ss >> e[0]; if(ss.fail()) return false;
				break;
			case 21:
				// end y
				get_line();
				ss.str(m_str); ss >> e[1]; if(ss.fail()) return false;
				break;
			case 31:
				// end z
				get_line();
				ss.str(m_str); ss >> e[2]; if(ss.fail()) return false;
				break;
			case 100:
			case 39:
			case 210:
			case 220:
			case 230:
				// skip the next line
				get_line();
				break;
			default:
				// skip the next line
				get_line();
				break;
		}

	}

	OnReadLine(s, e, undoably);
	return false;
}

bool CDxfRead::ReadArc(bool undoably)
{
	double start_angle = 0.0;// in degrees
	double end_angle = 0.0;
	double radius = 0.0;
	double c[3]; // centre

	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf_s(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with arc
				OnReadArc(start_angle, end_angle, radius, c, undoably);
				return true;
			case 10:
				// centre x
				get_line();
				ss.str(m_str); ss >> c[0]; if(ss.fail()) return false;
				break;
			case 20:
				// centre y
				get_line();
				ss.str(m_str); ss >> c[1]; if(ss.fail()) return false;
				break;
			case 30:
				// centre z
				get_line();
				ss.str(m_str); ss >> c[2]; if(ss.fail()) return false;
				break;
			case 40:
				// radius
				get_line();
				ss.str(m_str); ss >> radius; if(ss.fail()) return false;
				break;
			case 50:
				// start angle
				get_line();
				ss.str(m_str); ss >> start_angle; if(ss.fail()) return false;
				break;
			case 51:
				// end angle
				get_line();
				ss.str(m_str); ss >> end_angle; if(ss.fail()) return false;
				break;
			case 100:
			case 39:
			case 210:
			case 220:
			case 230:
				// skip the next line
				get_line();
				break;
			default:
				// skip the next line
				get_line();
				break;
		}
	}
	OnReadArc(start_angle, end_angle, radius, c, undoably);
	return false;
}

bool CDxfRead::ReadSpline(bool undoably)
{
	struct SplineData sd;
	sd.norm[0] = 0;
	sd.norm[1] = 0;
	sd.norm[2] = 1;
	sd.degree = 0;
	sd.knots = 0;
	sd.flag = 0;
	sd.control_points = 0;
	sd.fit_points = 0;

	double temp_double;
	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf_s(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with Spline
				//OnReadSpline(sd, undoably);
				return true;
			case 210:
				// normal x
				get_line();
				ss.str(m_str); ss >> sd.norm[0]; if(ss.fail()) return false;
				break;
			case 220:
				// normal y
				get_line();
				ss.str(m_str); ss >> sd.norm[1]; if(ss.fail()) return false;
				break;
			case 230:
				// normal z
				get_line();
				ss.str(m_str); ss >> sd.norm[2]; if(ss.fail()) return false;
				break;
			case 70:
				// flag
				get_line();
				ss.str(m_str); ss >> sd.flag; if(ss.fail()) return false;
				break;
			case 71:
				// degree
				get_line();
				ss.str(m_str); ss >> sd.degree; if(ss.fail()) return false;
				break;
			case 72:
				// knots
				get_line();
				ss.str(m_str); ss >> sd.knots; if(ss.fail()) return false;
				break;
			case 73:
				// control points
				get_line();
				ss.str(m_str); ss >> sd.control_points; if(ss.fail()) return false;
				break;
			case 74:
				// fit points
				get_line();
				ss.str(m_str); ss >> sd.fit_points; if(ss.fail()) return false;
				break;
			case 12:
				// starttan x
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.starttanx.push_back(temp_double);
				break;
			case 22:
				// starttan y
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.starttany.push_back(temp_double);
				break;
			case 32:
				// starttan z
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.starttanz.push_back(temp_double);
				break;
			case 13:
				// endtan x
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.endtanx.push_back(temp_double);
				break;
			case 23:
				// endtan y
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.endtany.push_back(temp_double);
				break;
			case 33:
				// endtan z
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.endtanz.push_back(temp_double);
				break;
			case 40:
				// knot
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.knot.push_back(temp_double);
				break;
			case 41:
				// weight
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.weight.push_back(temp_double);
				break;
			case 10:
				// control x
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.controlx.push_back(temp_double);
				break;
			case 20:
				// control y
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.controly.push_back(temp_double);
				break;
			case 30:
				// control z
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.controlz.push_back(temp_double);
				break;
			case 11:
				// fit x
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.fitx.push_back(temp_double);
				break;
			case 21:
				// fit y
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.fity.push_back(temp_double);
				break;
			case 31:
				// fit z
				get_line();
				ss.str(m_str); ss >> temp_double; if(ss.fail()) return false;
				sd.fitz.push_back(temp_double);
				break;
			case 42:
			case 43:
			case 44:
				// skip the next line
				get_line();
				break;
			default:
				// skip the next line
				get_line();
				break;
		}
	}
	//OnReadSpline(sd, undoably);
	return false;
}


bool CDxfRead::ReadCircle(bool undoably)
{
	double radius = 0.0;
	double c[3]; // centre

	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf_s(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with Circle
				OnReadCircle(c, radius, undoably);
				return true;
			case 10:
				// centre x
				get_line();
				ss.str(m_str); ss >> c[0]; if(ss.fail()) return false;
				break;
			case 20:
				// centre y
				get_line();
				ss.str(m_str); ss >> c[1]; if(ss.fail()) return false;
				break;
			case 30:
				// centre z
				get_line();
				ss.str(m_str); ss >> c[2]; if(ss.fail()) return false;
				break;
			case 40:
				// radius
				get_line();
				ss.str(m_str); ss >> radius; if(ss.fail()) return false;
				break;
			case 100:
			case 39:
			case 210:
			case 220:
			case 230:
				// skip the next line
				get_line();
				break;
			default:
				// skip the next line
				get_line();
				break;
		}
	}
	OnReadCircle(c, radius, undoably);
	return false;
}

bool CDxfRead::ReadEllipse(bool undoably)
{
	double c[3]; // centre
	double m[3]; //major axis point
	double ratio=0; //ratio of major to minor axis
	double start=0; //start of arc
	double end=0;  // end of arc

	while(!((*m_ifs).eof()))
	{
		get_line();
		int n;
		if(sscanf_s(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found, so finish with Ellipse
				OnReadEllipse(c, m, ratio, start, end, undoably);
				return true;
			case 10:
				// centre x
				get_line();
				ss.str(m_str); ss >> c[0]; if(ss.fail()) return false;
				break;
			case 20:
				// centre y
				get_line();
				ss.str(m_str); ss >> c[1]; if(ss.fail()) return false;
				break;
			case 30:
				// centre z
				get_line();
				ss.str(m_str); ss >> c[2]; if(ss.fail()) return false;
				break;
			case 11:
				// major x
				get_line();
				ss.str(m_str); ss >> m[0]; if(ss.fail()) return false;
				break;
			case 21:
				// major y
				get_line();
				ss.str(m_str); ss >> m[1]; if(ss.fail()) return false;
				break;
			case 31:
				// major z
				get_line();
				ss.str(m_str); ss >> m[2]; if(ss.fail()) return false;
				break;
			case 40:
				// ratio
				get_line();
				ss.str(m_str); ss >> ratio; if(ss.fail()) return false;
				break;
			case 41:
				// start
				get_line();
				ss.str(m_str); ss >> start; if(ss.fail()) return false;
				break;
			case 42:
				// end
				get_line();
				ss.str(m_str); ss >> end; if(ss.fail()) return false;
				break;	
			case 100:
			case 210:
			case 220:
			case 230:
				// skip the next line
				get_line();
				break;
			default:
				// skip the next line
				get_line();
				break;
		}
	}
	OnReadEllipse(c, m, ratio, start, end, undoably);
	return false;
}

#if 0
static bool poly_prev_found = false;
static double poly_prev_x;
static double poly_prev_y;
static double poly_prev_bulge_found;
static double poly_prev_bulge;
static bool poly_first_found = false;
static double poly_first_x;
static double poly_first_y;

static void AddPolyLinePoint(CDxfRead* dxf_read, double x, double y, bool bulge_found, double bulge, bool undoably)
{
	if(poly_prev_found)
	{
		bool arc_done = false;
		if(poly_prev_bulge_found)
		{
				double dx = x - poly_prev_x;
				double dy = y - poly_prev_y;
				double c = sqrt(dx*dx + dy*dy);

				double a = atan(fabs(poly_prev_bulge))*4;

				//find radius of circle that for arc of angle a, has chord length c
				double r = (c/2) / cos((Pi-a)/2);

				double d = sqrt(r*r - (c/2)*(c/2));
				
				double ps[3] = {poly_prev_x, poly_prev_y, 0};
				double pe[3] = {x, y, 0};
				gp_Pnt pPnt = make_point(ps);
				gp_Pnt nPnt = make_point(pe);
				gp_Dir dir(nPnt.XYZ()-pPnt.XYZ());
				
				gp_Pnt mid = pPnt.XYZ() + dir.XYZ() * c / 2;
				
				dir.Rotate(gp_Ax1(gp_Pnt(0,0,0),gp_Dir(0,0,1)),Pi/2);
				gp_Pnt off;
				if(poly_prev_bulge >= 0)
					off = mid.XYZ() + dir.XYZ() * (d); 
				else
					off = mid.XYZ() + dir.XYZ() * (-d); 
			
				double pc[3];
				extract(off,pc);
				
				dxf_read->OnReadArc(ps, pe, pc, poly_prev_bulge >= 0, undoably);
				arc_done = true;
			
		}

		if(!arc_done)
		{
			double s[3] = {poly_prev_x, poly_prev_y, 0};
			double e[3] = {x, y, 0};
			dxf_read->OnReadLine(s, e, undoably);
		}
	}

	poly_prev_found = true;
	poly_prev_x = x;
	poly_prev_y = y;
	if(!poly_first_found)
	{
		poly_first_x = x;
		poly_first_y = y;
		poly_first_found = true;
	}
	poly_prev_bulge_found = bulge_found;
	poly_prev_bulge = bulge;
}

static void PolyLineStart()
{
	poly_prev_found = false;
	poly_first_found = false;
}

bool CDxfRead::ReadLwPolyLine(bool undoably)
{
	PolyLineStart();

	bool x_found = false;
	bool y_found = false;
	double x = 0.0;
	double y = 0.0;
	bool bulge_found = false;
	double bulge = 0.0;
	bool closed = false;
	int flags;
	bool next_item_found = false;

	while(!((*m_ifs).eof()) && !next_item_found)
	{
		get_line();
		int n;
		if(sscanf_s(m_str, "%d", &n) != 1)return false;
		std::istringstream ss;
		ss.imbue(std::locale("C"));
		switch(n){
			case 0:
				// next item found
				if(x_found && y_found){
					// add point
					AddPolyLinePoint(this, x, y, bulge_found, bulge, undoably);
					bulge_found = false;
					x_found = false;
					y_found = false;
				}
				next_item_found = true;
				break;
			case 10:
				// x
				get_line();
				if(x_found && y_found){
					// add point
					AddPolyLinePoint(this, x, y, bulge_found, bulge, undoably);
					bulge_found = false;
					x_found = false;
					y_found = false;
				}
				ss.str(m_str); ss >> x; if(ss.fail()) return false;
				x_found = true;
				break;
			case 20:
				// y
				get_line();
				ss.str(m_str); ss >> y; if(ss.fail()) return false;
				y_found = true;
				break;
			case 42:
				// bulge
				get_line();
				ss.str(m_str); ss >> bulge; if(ss.fail()) return false;
				bulge_found = true;
				break;
			case 70:
				// flags
				get_line();
				if(sscanf_s(m_str, "%d", &flags) != 1)return false;
				closed = ((flags & 1) != 0);
				break;
			default:
				// skip the next line
				get_line();
				break;
		}
	}

	if(next_item_found)
	{
		if(closed && poly_first_found)
		{
			// repeat the first point
			AddPolyLinePoint(this, poly_first_x, poly_first_y, false, 0.0, undoably);
		}
		return true;
	}

	return false;
}
#endif

void CDxfRead::OnReadArc(double start_angle, double end_angle, double radius, const double* c, bool undoably){
	double s[3], e[3];
	s[0] = c[0] + radius * cos(start_angle * Pi/180);
	s[1] = c[1] + radius * sin(start_angle * Pi/180);
	s[2] = c[2];
	e[0] = c[0] + radius * cos(end_angle * Pi/180);
	e[1] = c[1] + radius * sin(end_angle * Pi/180);
	e[2] = c[2];

	OnReadArc(s, e, c, true, undoably);
}

void CDxfRead::OnReadCircle(const double* c, double radius, bool undoably){
	double s[3];
    double start_angle = 0;
	s[0] = c[0] + radius * cos(start_angle * Pi/180);
	s[1] = c[1] + radius * sin(start_angle * Pi/180);
	s[2] = c[2];

	OnReadCircle(s, c, false, undoably); //false to change direction because otherwise the arc length is zero
}

void CDxfRead::OnReadEllipse(const double* c, const double* m, double ratio, double start_angle, double end_angle, bool undoably){
	double major_radius = sqrt(m[0]*m[0] + m[1]*m[1] + m[2]*m[2]);
	double minor_radius = major_radius * ratio;

	//Since we only support 2d stuff, we can calculate the rotation from the major axis x and y value only,
	//since z is zero, major_radius is the vector length
	
	double rotation = atan2(m[1]/major_radius,m[0]/major_radius);


	OnReadEllipse(c, major_radius, minor_radius, rotation, start_angle, end_angle, true, undoably); 
}

void CDxfRead::get_line()
{
	m_ifs->getline(m_str, 1024);

	char str[1024];
	int len = strlen(m_str);
	int j = 0;
	bool non_white_found = false;
	for(int i = 0; i<len; i++){
		if(non_white_found || (m_str[i] != ' ' && m_str[i] != '\t')){
#if wxUSE_UNICODE
			if(m_str[i] != '\r')
#endif
			{
				str[j] = m_str[i]; j++;
			}
			non_white_found = true;
		}
	}
	str[j] = 0;
	strcpy_s(m_str, str);
}

void CDxfRead::DoRead(bool undoably)
{
	if(m_fail)return;

	get_line();

	while(!((*m_ifs).eof()))
	{
		if(!strcmp(m_str, "0"))
		{
			get_line();

			if(!strcmp(m_str, "LINE")){
				if(!ReadLine(undoably))return;
				continue;
			}
			else if(!strcmp(m_str, "ARC")){
				if(!ReadArc(undoably))return;
				continue;
			}
			else if(!strcmp(m_str, "CIRCLE")){
				if(!ReadCircle(undoably))return;
				continue;
			}
			else if(!strcmp(m_str, "ELLIPSE")){
				if(!ReadEllipse(undoably))return;
				continue;
			}
			else if(!strcmp(m_str, "SPLINE")){
				if(!ReadSpline(undoably))return;
				continue;
			}
			else if(!strcmp(m_str, "LWPOLYLINE")){
				//if(!ReadLwPolyLine(undoably))return;
				continue;
			}
		}

		get_line();
	}

	OnEndRead();
}

const double touch_toler = 0.001;

static bool touching(const double* p0, const double* p1)
{
	for(int i = 0; i<2; i++)
	{
		if(fabs(p0[i] - p1[i]) > touch_toler)return false;
	}

	return true;
}

void AreaDxfRead::OnReadVertex(const double* s, const CVertex& v)
{
	bool reverse_span = false;

	if(m_curve)
	{
		bool is_touching = false;

		if(touching(s, m_previous_end))
		{
			is_touching = true;
		}
		else if(touching(v.m_p, m_previous_end))
		{
			is_touching = true;
			reverse_span = true;
		}

		// if end point touching
		if(!is_touching)
		{
			// add curve
			m_area->m_curves.push_back(*m_curve);
			m_curve = NULL;
		}
	}

	if(m_curve == NULL)
	{
		// start a new curve
		m_curve = new CCurve();
		m_curve->m_vertices.push_back(CVertex(0, s[0], s[1], 0, 0));
	}

	// add to curve
	if(reverse_span)m_curve->m_vertices.push_back(CVertex(-v.m_type, s[0], s[1], v.m_c[0], v.m_c[1]));
	else m_curve->m_vertices.push_back(v);

	// remember end point
	memcpy(m_previous_end, v.m_p, 2*sizeof(double));
}

void AreaDxfRead::OnReadLine(const double* s, const double* e, bool undoably)
{
	OnReadVertex(s, CVertex(0, e[0], e[1], 0, 0));
}

void AreaDxfRead::OnReadArc(const double* s, const double* e, const double* c, bool dir, bool undoably)
{
	OnReadVertex(s, CVertex(dir? 1:-1, e[0], e[1], c[0], c[1]));
}

void AreaDxfRead::OnEndRead()
{
	if(m_curve)
	{
		// add curve
		m_area->m_curves.push_back(*m_curve);
		m_curve = NULL;
	}
}