// Path.h

// written by Dan Heeks, February 6th 2009, license: GPL version 3 http://www.gnu.org/licenses/gpl-3.0.txt

#pragma once

class CPath
{
public:
	virtual void OnRapidZ(double z) = 0;
	virtual void OnRapidXY(double x, double y) = 0;
	virtual void OnFeedZ(double z) = 0;
	virtual void OnFeedXY(double x, double y) = 0;
	virtual void OnArcCCW(double x, double y, double i, double j) = 0;
	virtual void OnArcCW(double x, double y, double i, double j) = 0;
};