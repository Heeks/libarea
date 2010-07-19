// Point.h

#pragma once

#include <cmath>

class Point{
public:
	double x, y;

	Point():x(0.0), y(0.0){}
	Point(double X, double Y):x(X), y(Y){}
	Point(const double* p):x(p[0]), y(p[1]){}

	const Point operator+(const Point& p)const{return Point(x + p.x, y + p.y);}
	const Point operator-(const Point& p)const{return Point(x - p.x, y - p.y);}
	const Point operator*(double d)const{return Point(x * d, y * d);}
	const Point operator/(double d)const{return Point(x / d, y / d);}
	double dist(const Point &p)const{double dx = p.x - x; double dy = p.y - y; return sqrt(dx*dx + dy*dy);}
};

const Point operator*(const double &d, const Point &p);
