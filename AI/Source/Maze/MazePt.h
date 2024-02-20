#ifndef _MAZEPT_H_
#define _MAZEPT_H_

class MazePt
{
public:
	int x;
	int y;
public:
	MazePt();
	MazePt(int _x, int _y);
	MazePt(const MazePt& rhs);
	~MazePt();

	void Set(const MazePt& rhs);
	void Set(int _x, int _y);

	bool IsEqual(const MazePt& rhs) const;
	bool DoesNotEqual(const MazePt& rhs) const;

	void Modify(int _dx, int _dy);
};

#endif

