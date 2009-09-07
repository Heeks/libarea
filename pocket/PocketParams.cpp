// PocketParams.cpp

// copyright Dan Heeks, September 3rd 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#include "stdafx.h"

#include "PocketParams.h"

PocketParams::PocketParams()
{
	m_from_center = true;
	m_round_corner_factor = 1.0;
	m_material_allowance = 0.1;
	m_step_over = 1.5;
	m_clearance_height = 5;
	m_start_depth = 0;
	m_step_down = 1;
	m_final_depth = -3;
	m_rapid_down_to_height = 2;
	m_tool_diameter = 3;
}
