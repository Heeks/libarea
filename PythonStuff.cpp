// PythonStuff.cpp

// written by Dan Heeks, February 6th 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#include "PythonStuff.h"

#include <set>

#include "Area.h"

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

#include "kbool/include/_lnk_itr.h"

#include "kbool/include/booleng.h"

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

static int span_number = 1;

static PyObject* area_add_point(PyObject* self, PyObject* args)
{
	double x, y, i, j;
	int sp, ik;
	if (!PyArg_ParseTuple(args, "iidddd", &ik, &sp, &x, &y, &i, &j)) return NULL;

	CArea* k = (CArea*)ik;
	if(valid_areas.find(k) != valid_areas.end())
	{
		// add a curve if there isn't one
		if(k->m_curves.size() == 0)k->m_curves.push_back(CCurve());
		int curve_index = k->m_curves.size() - 1;

		// can't add arc as first span
		if(sp && k->m_curves[curve_index].m_vertices.size() == 0){ const char* str = "can't add arc to area as first point"; printf(str); throw(str);}

		// add the vertex
		k->m_curves[curve_index].m_vertices.push_back(CVertex(sp, x, y, i, j, span_number));
		span_number++;
	}

	Py_RETURN_NONE;
}

static PyObject* area_start_new_curve(PyObject* self, PyObject* args)
{
	int ik;
	if (!PyArg_ParseTuple(args, "i", &ik)) return NULL;

	CArea* k = (CArea*)ik;
	if(valid_areas.find(k) != valid_areas.end())
	{
		// add a new curve
		k->m_curves.push_back(CCurve());
	}

	Py_RETURN_NONE;
}

static PyObject* area_offset(PyObject* self, PyObject* args)
{
	int ik;
	double inwards;
	if (!PyArg_ParseTuple(args, "id", &ik, &inwards)) return NULL;

	CArea* k = (CArea*)ik;
	//int ret = 0;

	if(valid_areas.find(k) != valid_areas.end())
	{
		k->Offset(inwards);
	}

	Py_RETURN_NONE;
}

static PyObject* area_subtract(PyObject* self, PyObject* args)
{
	int a1, a2;
	if (!PyArg_ParseTuple(args, "ii", &a1, &a2)) return NULL;

	CArea* area1 = (CArea*)a1;
	CArea* area2 = (CArea*)a2;

	if(valid_areas.find(area1) != valid_areas.end() && valid_areas.find(area2) != valid_areas.end())
	{
		area1->Subtract(*area2);
	}

	Py_RETURN_NONE;
}

static PyObject* area_set_round_corner_factor(PyObject* self, PyObject* args)
{
	if (!PyArg_ParseTuple(args, "d", &CArea::m_round_corners_factor)) return NULL;

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

static PyObject* area_add_curve(PyObject* self, PyObject* args)
{
	int ik, ij, ii;
	if (!PyArg_ParseTuple(args, "iii", &ik, &ij, &ii)) return NULL;

	CArea* k = (CArea*)ik;
	CArea* j = (CArea*)ij;
	if(valid_areas.find(k) != valid_areas.end() && valid_areas.find(j) != valid_areas.end())
	{
        if(ii >= 0 && ii < (int)j->m_curves.size()){
            // add curve
            k->m_curves.push_back(j->m_curves[ii]);
        }
	}

	Py_RETURN_NONE;
}

static PyMethodDef AreaMethods[] = {
	{"new", area_new, METH_VARARGS , ""},
	{"exists", area_exists, METH_VARARGS , ""},
	{"delete", area_delete, METH_VARARGS , ""},
	{"add_point", area_add_point, METH_VARARGS , ""},
	{"start_new_curve", area_start_new_curve, METH_VARARGS , ""},
	{"subtract", area_subtract, METH_VARARGS , ""},
	{"offset", area_offset, METH_VARARGS , ""},
	{"set_round_corner_factor", area_set_round_corner_factor, METH_VARARGS , ""},
	{"copy", area_copy, METH_VARARGS , ""},
	{"num_curves", area_num_curves, METH_VARARGS , ""},
	{"num_vertices", area_num_vertices, METH_VARARGS , ""},
	{"get_vertex", area_get_vertex, METH_VARARGS , ""},
	{"add_curve", area_add_curve, METH_VARARGS , ""},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initarea(void)
{
	Py_InitModule("area", AreaMethods);
}
