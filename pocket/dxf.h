// dxf.h

// copyright Dan Heeks, September 3rd 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

// copied from dxf.h in HeeksCAD

#pragma once
struct SplineData
{
	double norm[3];
	int degree;
	int knots;
	int control_points;
	int fit_points;
	int flag;
	std::list<double> starttanx;
	std::list<double> starttany;
	std::list<double> starttanz;
	std::list<double> endtanx;
	std::list<double> endtany;
	std::list<double> endtanz;
	std::list<double> knot;
	std::list<double> weight;
	std::list<double> controlx;
	std::list<double> controly;
	std::list<double> controlz;
	std::list<double> fitx;
	std::list<double> fity;
	std::list<double> fitz;
};

class CDxfWrite{
private:
	ofstream* m_ofs;
	bool m_fail;

public:
	CDxfWrite(const _TCHAR* filepath);
	~CDxfWrite();

	bool Failed(){return m_fail;}

	void WriteLine(const double* s, const double* e);
	void WriteArc(const double* s, const double* e, const double* c, bool dir);
    void WriteEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir);
	void WriteCircle(const double* c, double radius);
};

// derive a class from this and implement it's virtual functions
class CDxfRead{
private:
	ifstream* m_ifs;

	bool m_fail;
	char m_str[1024];

	bool ReadLine(bool undoably);
	bool ReadArc(bool undoably);
	bool ReadCircle(bool undoably);
	bool ReadEllipse(bool undoably);
	bool ReadSpline(bool undoably);
	void OnReadArc(double start_angle, double end_angle, double radius, const double* c, bool undoably);
	void OnReadCircle(const double* c, double radius, bool undoably);
    void OnReadEllipse(const double* c, const double* m, double ratio, double start_angle, double end_angle, bool undoably);
	void get_line();

public:
	CDxfRead(const _TCHAR* filepath); // this opens the file
	~CDxfRead(); // this closes the file

	bool Failed(){return m_fail;}
	void DoRead(bool undoably); // this reads the file and calls the following functions

	virtual void OnReadLine(const double* s, const double* e, bool undoably){}
	virtual void OnReadArc(const double* s, const double* e, const double* c, bool dir, bool undoably){}
	virtual void OnReadCircle(const double* s, const double* c, bool dir, bool undoably){}
	virtual void OnReadEllipse(const double* c, double major_radius, double minor_radius, double rotation, double start_angle, double end_angle, bool dir, bool undoably){}
	virtual void OnEndRead(){}
};

class CArea;
class CCurve;
class CVertex;

class AreaDxfRead : public CDxfRead{
	CArea* m_area;
	CCurve* m_curve;
	double m_previous_end[2];

	void OnReadVertex(const double* s, const CVertex& v);

public:
	AreaDxfRead(const _TCHAR* filepath, CArea* a):CDxfRead(filepath), m_area(a), m_curve(NULL){}

	// CDxfRead's virtual functions
	void OnReadLine(const double* s, const double* e, bool undoably);
	void OnReadArc(const double* s, const double* e, const double* c, bool dir, bool undoably);
	void OnEndRead();
};
