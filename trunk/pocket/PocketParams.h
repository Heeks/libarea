// PocketParams.h

// copyright Dan Heeks, September 3rd 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#pragma once

class PocketParams
{
public:
	bool m_from_center;
	double m_round_corner_factor;
	double m_material_allowance;
	double m_step_over;
	double m_clearance_height;
	double m_start_depth;
	double m_step_down;
	double m_final_depth;
	double m_rapid_down_to_height;
	double m_tool_diameter;
	int m_format_style;

	PocketParams();
};