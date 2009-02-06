// PythonStuff.cpp

#include "PythonStuff.h"

#include <set>

#include "Area.h"

#if _DEBUG
#undef _DEBUG
#include <python.h>
#define _DEBUG
#else
#include <python.h>
#endif


#ifdef __GNUG__
#pragma implementation
#endif

#include "kbool/include/_lnk_itr.h"

#include "kbool/include/booleng.h"

void ArmBoolEng( Bool_Engine* booleng )
{
    // set some global vals to arm the boolean engine
    double DGRID = 10;  // round coordinate X or Y value in calculations to this
    double MARGE = 0.00001;   // snap with in this range points to lines in the intersection routines
                          // should always be > DGRID  a  MARGE >= 10*DGRID is oke
                          // this is also used to remove small segments and to decide when
                          // two segments are in line.
    double CORRECTIONFACTOR = 5.000;  // correct the polygons by this number
    double CORRECTIONABER   = 0.010;    // the accuracy for the rounded shapes used in correction
    double ROUNDFACTOR      = 0.015;    // when will we round the correction shape to a circle
    double SMOOTHABER       = 0.100;   // accuracy when smoothing a polygon
    double MAXLINEMERGE     = 10.000; // leave as is, segments of this length in smoothen
 

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

std::set<CArea*> valid_areas;

static PyObject* area_new(PyObject* self, PyObject* args)
{
	CArea* new_object = new CArea();
	valid_areas.insert(new_object);

	// return new object cast to an int
	PyObject *pValue = PyInt_FromLong((long)new_object);
	Py_INCREF(pValue);
	return pValue;
}

static PyObject* area_exists(PyObject* self, PyObject* args)
{
	int ik;
	if (!PyArg_ParseTuple(args, "i", &ik)) return NULL;

	CArea* k = (CArea*)ik;
	bool exists = (valid_areas.find(k) != valid_areas.end());

	// return exists
	PyObject *pValue = exists ? Py_True : Py_False;
	Py_INCREF(pValue);
	return pValue;
}

static PyObject* area_delete(PyObject* self, PyObject* args)
{
	int ik;
	if (!PyArg_ParseTuple(args, "i", &ik)) return NULL;

	CArea* k = (CArea*)ik;
	if(valid_areas.find(k) != valid_areas.end())
	{
		delete k;
		valid_areas.erase(k);
	}

	Py_RETURN_NONE;
}

static PyObject* area_add_point(PyObject* self, PyObject* args)
{
	double x, y, i, j;
	int sp, ik;
	if (!PyArg_ParseTuple(args, "iidddd", &ik, &sp, &x, &y, &i, &j)) return NULL;

	CArea* k = (CArea*)ik;
	if(valid_areas.find(k) != valid_areas.end())
	{
		if(k->m_curves.size() == 0)k->m_curves.push_back(CCurve());

		CVertex spv(sp, x, y, i, j);
#if 0
		// to do, arcs
		if(sp)
		{
			// can't add arc as first span
			if(!(k->m_curves[0].m_vertices.size() == 0)){ const char* str = "can't add arc to area as first point"; cout << str; throw(str);}

			// fix radius by moving centre point a little bit
			int previous_vertex = k->nSpans();
			spVertex pv;
			k->Get(previous_vertex, pv);
			Vector2d v(spv.pc, pv.p);
			double r = v.magnitude();
			Circle c1( pv.p, r );
			Circle c2( spv.p, r );
			Point leftInters, rightInters;
			if(c1.Intof(c2, leftInters, rightInters) == 2)
			{
				double d1 = spv.pc.Dist(leftInters);
				double d2 = spv.pc.Dist(rightInters);
				if(d1<d2)spv.pc = leftInters;
				else spv.pc = rightInters;
			}
		}
#endif
		k->m_curves[0].m_vertices.push_back(spv);
	}

	Py_RETURN_NONE;
}

static PyObject* area_offset(PyObject* self, PyObject* args)
{
	int ik;
	double inwards;
	if (!PyArg_ParseTuple(args, "id", &ik, &inwards)) return NULL;

	CArea* k = (CArea*)ik;
	int ret = 0;

	if(valid_areas.find(k) != valid_areas.end())
	{
		k->Offset(inwards);
	}

	Py_RETURN_NONE;
}

static PyObject* area_copy(PyObject* self, PyObject* args)
{
	int ik;
	int ik2;
	if (!PyArg_ParseTuple(args, "ii", &ik, &ik2)) return NULL;
	CArea* k = (CArea*)ik;
	CArea* k2 = (CArea*)ik2;
	if(valid_areas.find(k) != valid_areas.end())
	{
		if(valid_areas.find(k2) != valid_areas.end())
		{
			*k2 = *k;
		}
	}

	Py_RETURN_NONE;
}

static PyObject* area_num_curves(PyObject* self, PyObject* args)
{
	int ik;
	if (!PyArg_ParseTuple(args, "i", &ik)) return NULL;
	CArea* k = (CArea*)ik;

	int n = 0;
	if(valid_areas.find(k) != valid_areas.end())
	{
		n = k->m_curves.size();
	}

	PyObject *pValue = PyInt_FromLong(n);
	Py_INCREF(pValue);
	return pValue;
}

static PyObject* area_num_vertices(PyObject* self, PyObject* args)
{
	int ik, ic;
	if (!PyArg_ParseTuple(args, "ii", &ik, &ic)) return NULL;
	CArea* k = (CArea*)ik;

	int n = 0;
	if(valid_areas.find(k) != valid_areas.end())
	{
		if(ic >= 0 && ic < (int)(k->m_curves.size()))n = k->m_curves[ic].m_vertices.size();
	}

	PyObject *pValue = PyInt_FromLong(n);
	Py_INCREF(pValue);
	return pValue;
}

static PyObject* area_get_vertex(PyObject* self, PyObject* args)
{
	int ik, ic;
	int index; // 0 is first
	if (!PyArg_ParseTuple(args, "iii", &ik, &ic, &index)) return NULL;
	CArea* k = (CArea*)ik;

	int sp = 0;
	double x = 0.0, y = 0.0;
	double cx = 0.0, cy = 0.0;

	if(valid_areas.find(k) != valid_areas.end())
	{
		if(ic >= 0 && ic < (int)(k->m_curves.size()))
		{
			CCurve& curve = k->m_curves[ic];
			if(index >= 0 && index < (int)(curve.m_vertices.size()))
			{
				CVertex& vertex = curve.m_vertices[index];
				sp = vertex.m_type;
				x = vertex.m_p[0];
				y = vertex.m_p[1];
				cx = vertex.m_c[0];
				cy = vertex.m_c[1];
			}
		}
	}

	// return span data as a tuple
	PyObject *pTuple = PyTuple_New(5);
	{
		PyObject *pValue = PyInt_FromLong(sp);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 0, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(x);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 1, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(y);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 2, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(cx);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 3, pValue);
	}
	{
		PyObject *pValue = PyFloat_FromDouble(cy);
		if (!pValue){
			Py_DECREF(pTuple);return NULL;
		}
		PyTuple_SetItem(pTuple, 4, pValue);
	}

	Py_INCREF(pTuple);
	return pTuple;
}

static PyMethodDef AreaMethods[] = {
	{"new", area_new, METH_VARARGS , ""},
	{"exists", area_exists, METH_VARARGS , ""},
	{"delete", area_delete, METH_VARARGS , ""},
	{"add_point", area_add_point, METH_VARARGS , ""},
	{"offset", area_offset, METH_VARARGS , ""},
	{"copy", area_copy, METH_VARARGS , ""},
	{"num_curves", area_num_curves, METH_VARARGS , ""},
	{"num_vertices", area_num_vertices, METH_VARARGS , ""},
	{"get_vertex", area_get_vertex, METH_VARARGS , ""},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initarea(void)
{
	Py_InitModule("area", AreaMethods);
}
