#include "MazePt.h"

MazePt::MazePt()
{
	x = 0;
	y = 0;
}

MazePt::MazePt(int _x, int _y)
{
	x = _x;
	y = _y;
}

MazePt::MazePt(const MazePt& rhs)
{
	x = rhs.x;
	y = rhs.y;
}

MazePt::~MazePt()
{
}

void MazePt::Set(const MazePt& rhs)
{
	x = rhs.x;
	y = rhs.y;
}

void MazePt::Set(int _x, int _y)
{
	x = _x;
	y = _y;
}

bool MazePt::IsEqual(const MazePt& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

bool MazePt::DoesNotEqual(const MazePt& rhs) const
{
	return x != rhs.x || y != rhs.y;
}

void MazePt::Modify(int _dx, int _dy)
{
	x += _dx;
	y += _dy;
}

