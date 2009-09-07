// TextPath.cpp

// copyright Dan Heeks, September 3rd 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#include "stdafx.h"
#include "TextPath.h"

CTextPath::CTextPath(const _TCHAR* txt_file)
{
	m_ofs = new ofstream(Ttc(txt_file));
}

CTextPath::~CTextPath()
{
	delete m_ofs;
}

bool CTextPath::Failed()
{
	return !(*m_ofs);
}

static char str_for_num[1024];
static const char* num(double v)
{
	sprintf(str_for_num, "%g", v);
	return str_for_num;
}

void CTextPath::OnRapidZ(double z)
{
	(*m_ofs)<<"G0Z"<<num(z)<<"\n";
}

void CTextPath::OnRapidXY(double x, double y)
{
	(*m_ofs)<<"G0X"<<num(x);
	(*m_ofs)<<"Y"<<num(y)<<"\n";
}

void CTextPath::OnFeedZ(double z)
{
	(*m_ofs)<<"G1Z"<<num(z)<<"\n";
}

void CTextPath::OnFeedXY(double x, double y)
{
	(*m_ofs)<<"G1X"<<num(x);
	(*m_ofs)<<"Y"<<num(y)<<"\n";
}

void CTextPath::OnArcCCW(double x, double y, double i, double j)
{
	(*m_ofs)<<"G3X"<<num(x);
	(*m_ofs)<<"Y"<<num(y);
	(*m_ofs)<<"I"<<num(i);
	(*m_ofs)<<"J"<<num(j)<<"\n";
}

void CTextPath::OnArcCW(double x, double y, double i, double j)
{
	(*m_ofs)<<"G2X"<<num(x);
	(*m_ofs)<<"Y"<<num(y);
	(*m_ofs)<<"I"<<num(i);
	(*m_ofs)<<"J"<<num(j)<<"\n";
}

