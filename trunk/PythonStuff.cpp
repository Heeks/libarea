// PythonStuff.cpp

// written by Dan Heeks, February 6th 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#include "PythonStuff.h"

#include "Area.h"
#include "Point.h"

#if _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#ifdef __GNUG__
#pragma implementation
#endif

#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/call.hpp>

namespace bp = boost::python;

boost::python::list getVertices(const CCurve& curve) {
	boost::python::list vlist;
	BOOST_FOREACH(const CVertex& vertex, curve.m_vertices) {
		vlist.append(vertex);
    }
	return vlist;
}

boost::python::list getCurves(const CArea& area) {
	boost::python::list clist;
	BOOST_FOREACH(const CCurve& curve, area.m_curves) {
		clist.append(curve);
    }
	return clist;
}

static void print_curve(const CCurve& c)
{
	unsigned int nvertices = c.m_vertices.size();
	printf("number of vertices = %d\n", nvertices);
	int i = 0;
	for(std::list<CVertex>::const_iterator It = c.m_vertices.begin(); It != c.m_vertices.end(); It++, i++)
	{
		const CVertex& vertex = *It;
		printf("vertex %d type = %d, x = %g, y = %g", i+1, vertex.m_type, vertex.m_p.x / CArea::m_units, vertex.m_p.y / CArea::m_units);
		if(vertex.m_type)printf(", xc = %g, yc = %g", vertex.m_c.x / CArea::m_units, vertex.m_c.y / CArea::m_units);
		printf("\n");
	}
}

static void print_area(const CArea &a)
{
	for(std::list<CCurve>::const_iterator It = a.m_curves.begin(); It != a.m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		print_curve(curve);
	}
}

static void set_round_corner_factor(double factor)
{
	CArea::m_round_corners_factor = factor;
}

static void set_units(double units)
{
	CArea::m_units = units;
}

BOOST_PYTHON_MODULE(area) {
	bp::class_<Point>("Point") 
        .def(bp::init<double, double>())
        .def(bp::init<Point>())
        .def(bp::other<double>() * bp::self)
        .def(bp::self * bp::other<double>())
        .def(bp::self - bp::other<Point>())
        .def(bp::self + bp::other<Point>())
        .def("dist", &Point::dist)
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
    ;

	bp::class_<CVertex>("Vertex") 
        .def(bp::init<CVertex>())
        .def(bp::init<int, Point, Point>())
        .def(bp::init<int, Point, Point, int>())
        .def_readwrite("type", &CVertex::m_type)
        .def_readwrite("p", &CVertex::m_p)
        .def_readwrite("c", &CVertex::m_c)
        .def_readwrite("user_data", &CVertex::m_user_data)
    ;

	bp::class_<CCurve>("Curve") 
        .def(bp::init<CCurve>())
        .def("getVertices", &getVertices)
        .def("append",&CCurve::append)
        .def("print", &print_curve)
    ;

	bp::class_<CArea>("Area") 
        .def(bp::init<CArea>())
        .def("getCurves", &getCurves)
        .def("append",&CArea::append)
        .def("Subtract",&CArea::Subtract)
        .def("Offset",&CArea::Offset)
        .def("FitArcs",&CArea::FitArcs)
        .def("print", &print_area)
		.def("num_curves", &CArea::num_curves)
    ;

    bp::def("set_round_corner_factor", set_round_corner_factor);
    bp::def("set_units", set_units);
}
