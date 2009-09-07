// pocket.cpp : Defines the entry point for the console application.
//
// written by Dan Heeks, September 3rd 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#include "stdafx.h"
#include "PocketParams.h"
#include "../Area.h"
#include "dxf.h"
#include "TextPath.h"

_TCHAR dxf_file[1024] = _T("input.dxf");
_TCHAR txt_file[1024] = _T("output.txt");

static PocketParams g_params;
static CPath* g_path = NULL;

void cut_area(const CArea& a)
{
	for(unsigned int i = 0; i< a.m_curves.size(); i++)
	{
		const CCurve& c = a.m_curves[i];

		double px = 0.0;
		double py = 0.0;

		for(unsigned int j = 0; j< c.m_vertices.size(); j++)
		{
			const CVertex& v = c.m_vertices[j];
			if(j == 0)
			{
				// rapid across
				g_path->OnRapidXY(v.m_p[0], v.m_p[1]);

				//rapid down
                g_path->OnRapidZ(g_params.m_rapid_down_to_height);
                
                //feed down
                g_path->OnFeedZ(g_params.m_final_depth);
			}
			else
			{
				if(v.m_type == 1)
				{
                    g_path->OnArcCCW(v.m_p[0], v.m_p[1], v.m_c[0] - px, v.m_c[1] - py);
				}
				else if(v.m_type == -1)
				{
                    g_path->OnArcCW(v.m_p[0], v.m_p[1], v.m_c[0] - px, v.m_c[1] - py);
				}
				else
				{
					g_path->OnFeedXY(v.m_p[0], v.m_p[1]);
				}
			}

            px = v.m_p[0];
            py = v.m_p[1];
		}

		g_path->OnRapidZ(g_params.m_clearance_height);
	}
}

void recur(std::list<const CArea*> &arealist, const CArea* a1)
{
	if(a1->m_curves.size() == 0)
		return;
    
    if(g_params.m_from_center)
        arealist.push_front(a1);
    else
        arealist.push_back(a1);

	CArea* a_offset = new CArea(*a1);
	a_offset->Offset(g_params.m_step_over);

	for(unsigned int i = 0; i<a_offset->m_curves.size(); i++)
	{
		CArea* a2 = new CArea();
		a2->m_curves.push_back(a_offset->m_curves[i]);
		recur(arealist, a2);
	}
}

void pocket_area(const CArea& area, const PocketParams& params, CPath& path)
{
	g_params = params;
	g_path = &path;

	if(g_params.m_rapid_down_to_height > g_params.m_clearance_height)
		g_params.m_rapid_down_to_height = g_params.m_clearance_height;
    
	CArea::m_round_corners_factor = params.m_round_corner_factor;

	std::list<const CArea*> arealist;

    CArea* a_firstoffset = new CArea(area);
	double first_offset = params.m_tool_diameter * 0.5 + params.m_material_allowance;
	a_firstoffset->Offset(first_offset);
    
    recur(arealist, a_firstoffset);
    
    int layer_count = int((params.m_start_depth - params.m_final_depth) / params.m_step_down);

    if(layer_count * params.m_step_down + 0.00001 < params.m_start_depth - params.m_final_depth)
        layer_count++;

	for(int i = 1; i< layer_count+1; i++)
	{
		double depth = params.m_final_depth;
        if(i != layer_count)
            depth = params.m_start_depth - i * params.m_step_down;

		for(std::list<const CArea*>::iterator It = arealist.begin(); It != arealist.end(); It++)
		{
			const CArea* a = *It;
			cut_area(*a);
		}
	}
}

void pocket_files(const _TCHAR* dxf_file, const _TCHAR* txt_file, const PocketParams &params)
{
	// this reads the dxf file and creates a text file with the pocketing path in

	// read the dxf file into a CArea object
	CArea area;
	AreaDxfRead dxf_read(dxf_file, &area);
	dxf_read.DoRead(false);
	if(dxf_read.Failed())
	{
		printf("dxf read failed, opening file %s", Ttc(dxf_file));
		return;
	}

	// open a text file to write to
	CTextPath text_path(txt_file);
	if(text_path.Failed())
	{
		printf("text write failed, opening file %s", Ttc(txt_file));
		return;
	}

	// make pocketing path
	pocket_area(area, params, text_path);
}

int _tmain(int argc, _TCHAR* argv[])
{
	// start of executable

	// default pocket parameters
	PocketParams params;

	// parse the command line parameters
	for(int i = 1; i < argc; i++)
	{
		_TCHAR* arg = argv[i];

		if(arg[0] == _T('-'))
		{
			i++;
			if(i < argc)
			{
				switch(arg[1])
				{
				case _T('f'):
					wcscpy_s(dxf_file, argv[i]);
					break;
				case _T('o'):
					wcscpy_s(txt_file, argv[i]);
					break;
				case _T('r'):
					if(swscanf(argv[i], _T("%lf"), &params.m_round_corner_factor)!=1){wprintf(_T("couldn't read value for -r"));return 0;}
					break;
				case _T('m'):
					if(swscanf(argv[i], _T("%lf"), &params.m_material_allowance)!=1){wprintf(_T("couldn't read value for -r"));return 0;}
					break;
				case _T('s'):
					if(swscanf(argv[i], _T("%lf"), &params.m_step_over)!=1){wprintf(_T("couldn't read value for -r"));return 0;}
					break;
				case _T('c'):
					if(swscanf(argv[i], _T("%lf"), &params.m_clearance_height)!=1){wprintf(_T("couldn't read value for -r"));return 0;}
					break;
				case _T('t'):
					if(swscanf(argv[i], _T("%lf"), &params.m_start_depth)!=1){wprintf(_T("couldn't read value for -r"));return 0;}
					break;
				case _T('i'):
					if(swscanf(argv[i], _T("%lf"), &params.m_step_down)!=1){wprintf(_T("couldn't read value for -r"));return 0;}
					break;
				case _T('z'):
					if(swscanf(argv[i], _T("%lf"), &params.m_final_depth)!=1){wprintf(_T("couldn't read value for -r"));return 0;}
					break;
				case _T('q'):
					if(swscanf(argv[i], _T("%lf"), &params.m_rapid_down_to_height)!=1){wprintf(_T("couldn't read value for -r"));return 0;}
					break;
				case _T('d'):
					if(swscanf(argv[i], _T("%lf"), &params.m_tool_diameter)!=1){wprintf(_T("couldn't read value for -r"));return 0;}
					break;
				case _T('n'):
					if(argv[i][0] == _T('c')) params.m_from_center = true;
					else params.m_from_center = false;
					break;
				}
			}
		}
	}

	pocket_files(dxf_file, txt_file, params);
}

