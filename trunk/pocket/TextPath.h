// TextPath.h

#pragma once

#include "Path.h"

class CTextPath: public CPath
{
	ofstream* m_ofs;

public:
	CTextPath(const _TCHAR* txt_file);
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
