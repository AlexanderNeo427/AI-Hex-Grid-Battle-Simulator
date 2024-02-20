#ifndef _MAZE_H_
#define _MAZE_H_

#include <vector>
#include "Tile.h"

using Grid = std::vector<Tile*>;

class Maze
{
public:
	enum class DIRECTION	// Has to be ordered properly
	{
		UP,
		RIGHTUP,
		RIGHTDOWN,
		DOWN,
		LEFTDOWN,
		LEFTUP,

		TOTAL,
	};
public:
	Maze(int _numGridX, int _numGridY, Tile::CONTENT _defaultTileContent);
	~Maze();

	void Reset(const Tile::CONTENT _tileContent);

	Grid GetGrid() const;
	Tile* GetTile(const MazePt& mazePt) const;
	Tile* GetTile(int x, int y) const;
	Tile* GetTile(int idx) const;
	Tile* GetNextTile(const MazePt &curr, const Maze::DIRECTION &dir) const;

	Tile::CONTENT See(const MazePt& mazePt) const;
	Tile::CONTENT See(int x, int y) const;
	Tile::CONTENT See(int idx) const;
	Tile::CONTENT SeeNext(const MazePt& curr, const Maze::DIRECTION &dir) const;

	Vector3 GetWorldPoint(const MazePt& mazePt) const;
	Vector3 GetWorldPoint(int x, int y) const;

	Maze::DIRECTION GetNextDir(const Maze::DIRECTION dir) const;
	Maze::DIRECTION GetPrevDir(const Maze::DIRECTION dir) const;
	int GetDirDiff(Maze::DIRECTION dir1, Maze::DIRECTION dir2) const;

	Maze::DIRECTION GetMoveDir(const Tile* pStartTile, const Tile* pTargetTile) const;
	Maze::DIRECTION GetMoveDir(const MazePt& startPt, const MazePt& endPt) const;

	void ResetPathfindingCosts();
	float ManhattanDistance(const Tile* pTile1, const Tile* pTile2) const;
	float ManhattanDistance(const MazePt& p1, const MazePt& p2) const;
private:
	const int m_numTilesX;
	const int m_numTilesY;

	Grid m_grid;
};

#endif

