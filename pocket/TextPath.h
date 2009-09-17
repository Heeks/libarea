// TextPath.h

#pragma once

#include "Path.h"

class CTextPath: public CPath
{
	ofstream* m_ofs;
	const char* num(double v);

public:
	int m_number_format;// 0 is like "3.141529", 1 is like "+00003.141"

	CTextPath(const _TCHAR* txt_file, int format_style);
	~CTextPath();

	bool Failed();

	// CPath's virtual functions
	void OnRapidZ(double z);
	void OnRapidXY(double x, double y);
	void OnFeedZ(double z);
	void OnFeedXY(double x, double y);
	void OnArcCCW(double x, double y, double i, double j);
	void OnArcCW(double x, double y, double i, double j);
};
