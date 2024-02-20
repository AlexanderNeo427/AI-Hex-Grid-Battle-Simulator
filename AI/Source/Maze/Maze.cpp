#include "Maze.h"
#include "../SceneData.h"

static SceneData& data = *SceneData::GetInstance();

Maze::Maze(int _numGridX, int _numGridY, Tile::CONTENT _defaultTileContent)
	:
	m_numTilesX( _numGridX ),
	m_numTilesY( _numGridY )
{
	int totalTiles = m_numTilesX * m_numTilesY;

	for (int i = 0; i < totalTiles; ++i)
	{
		int tileX = i % m_numTilesX;
		int tileY = i / m_numTilesX;

		Tile* tile = new Tile(_defaultTileContent,
							  MazePt(tileX, tileY), 
							  Transform::MazeToWorldPt(MazePt(tileX, tileY)));

		m_grid.push_back( tile );
	}
}

Maze::~Maze()
{
	for (Tile* &tile : m_grid)
	{
		if (tile != nullptr)
		{
			delete tile;
			tile = nullptr;
		}
	}
}

void Maze::Reset(const Tile::CONTENT _tileContent)
{
	for (Tile* &tile : m_grid)
		tile->SetContent( _tileContent );
}

Grid Maze::GetGrid() const
{
	return m_grid;
}

Tile* Maze::GetTile(const MazePt& mazePt) const
{
	if (mazePt.x < 0 || mazePt.x >= data.GetNumGrid() ||
		mazePt.y < 0 || mazePt.y >= data.GetNumGrid())
	{
		return nullptr;
	}

	return m_grid[(mazePt.y * data.GetNumGrid()) + mazePt.x];
}

Tile* Maze::GetTile(int x, int y) const
{
	if (x < 0 || x >= data.GetNumGrid() ||
		y < 0 || y >= data.GetNumGrid())
	{
		return nullptr;
	}

	return m_grid[(y * data.GetNumGrid()) + x];
}

Tile* Maze::GetTile(int idx) const
{
	if (idx < 0 || idx >= m_grid.size())
		return nullptr;

	return m_grid[idx];
}

Tile* Maze::GetNextTile(const MazePt& curr, const Maze::DIRECTION& dir) const
{
	MazePt temp = curr;

	switch ( dir )
	{
		case DIRECTION::UP: temp.Modify(0, 1); break;
		case DIRECTION::DOWN: temp.Modify(0, -1); break;
		case DIRECTION::LEFTUP: temp.x % 2 == 0 ? temp.Modify(-1, 0) : temp.Modify(-1, 1); break;
		case DIRECTION::LEFTDOWN: temp.x % 2 == 0 ? temp.Modify(-1, -1) : temp.Modify(-1, 0); break;
		case DIRECTION::RIGHTUP: temp.x % 2 == 0 ? temp.Modify(1, 0) : temp.Modify(1, 1); break;
		case DIRECTION::RIGHTDOWN: temp.x % 2 == 0 ? temp.Modify(1, -1) : temp.Modify(1, 0); break;
	}

	return GetTile( temp );
}

Tile::CONTENT Maze::See(const MazePt& mazePt) const
{
	if (mazePt.x < 0 || mazePt.x >= data.GetNumGrid() ||
		mazePt.y < 0 || mazePt.y >= data.GetNumGrid())
	{
		return Tile::CONTENT::WALL;
	}

	return m_grid[Transform::Grid2Dto1D(mazePt)]->GetContent();
}

Tile::CONTENT Maze::See(int x, int y) const
{
	if (x < 0 || x >= data.GetNumGrid() ||
		y < 0 || y >= data.GetNumGrid())
	{
		return Tile::CONTENT::WALL;
	}

	return m_grid[Transform::Grid2Dto1D(x, y)]->GetContent();
}

Tile::CONTENT Maze::See(int idx) const
{
	if (idx < 0 || idx >= m_grid.size())
	{
		return Tile::CONTENT::WALL;
	}

	return m_grid[ idx ]->GetContent();
}

Tile::CONTENT Maze::SeeNext(const MazePt &curr, const Maze::DIRECTION &dir) const
{
	return GetNextTile(curr, dir)->GetContent();
}

Vector3 Maze::GetWorldPoint(const MazePt& mazePt) const
{
	if (mazePt.x < 0 || mazePt.x >= data.GetNumGrid() ||
		mazePt.y < 0 || mazePt.y >= data.GetNumGrid())
	{
		throw "Can't access that element in the maze, out of range";
	}

	return m_grid[Transform::Grid2Dto1D(mazePt)]->GetWorldPt();
}

Vector3 Maze::GetWorldPoint(int x, int y) const
{
	if (x < 0 || x >= data.GetNumGrid() ||
		y < 0 || y >= data.GetNumGrid())
	{
		throw "Can't access that element in the maze, out of range";
	}

	return m_grid[Transform::Grid2Dto1D(x, y)]->GetWorldPt();
}

Maze::DIRECTION Maze::GetNextDir(const Maze::DIRECTION dir) const
{
	int maxDir = static_cast<int>( Maze::DIRECTION::TOTAL );
	int numDir = static_cast<int>( dir );

	if (numDir == maxDir - 1)
		return static_cast<Maze::DIRECTION>( 0 );

	return static_cast<Maze::DIRECTION>( numDir + 1 );
}

Maze::DIRECTION Maze::GetPrevDir(const Maze::DIRECTION dir) const
{
	int maxDir = static_cast<int>( Maze::DIRECTION::TOTAL );
	int numDir = static_cast<int>( dir );

	if (numDir == 0)
		return static_cast<Maze::DIRECTION>( maxDir - 1 );

	return static_cast<Maze::DIRECTION>( numDir - 1 );
}

int Maze::GetDirDiff(Maze::DIRECTION currDir, const Maze::DIRECTION targetDir) const
{
	if (currDir == targetDir)
		return 0;

	int leftCounter = 0;
	int rightCounter = 0;

	Maze::DIRECTION tempDir = currDir;
	while (tempDir != targetDir)
	{
		tempDir = GetPrevDir( tempDir );
		++leftCounter;
	}

	tempDir = currDir;
	while (tempDir != targetDir)
	{
		tempDir = GetNextDir( tempDir );
		++rightCounter;
	}

	return Math::Min( leftCounter, rightCounter );
}

Maze::DIRECTION Maze::GetMoveDir(const Tile* pStartTile, const Tile* pTargetTile) const
{
	MazePt startPt = pStartTile->GetMazePt();
	MazePt targetPt = pTargetTile->GetMazePt();

	Maze::DIRECTION bestDir;
	float minDist = FLT_MAX;

	int maxDir = static_cast<int>( Maze::DIRECTION::TOTAL );

	for (int i = 0; i < maxDir; ++i)
	{
		Maze::DIRECTION dir = static_cast<Maze::DIRECTION>(i);
		
		Tile* pNextTile = GetNextTile(startPt, dir);
		if (pNextTile == nullptr) continue;
		MazePt nextPt = pNextTile->GetMazePt();

		if (nextPt.IsEqual( targetPt ))
			return dir;

		//float dist = ManhattanDistance(pStartTile, pNextTile);
		Vector3 p1 = pTargetTile->GetWorldPt();
		Vector3 p2 = pNextTile->GetWorldPt();
		float dist = (p1 - p2).Length();

		if (dist < minDist)
		{
			minDist = dist;
			bestDir = dir;
		}
	}

	return bestDir;
}

Maze::DIRECTION Maze::GetMoveDir(const MazePt& startPt, const MazePt& endPt) const
{
	/*
	* Possble new implementation:
	*	(target - source).Dot( nextTile - source );
	*	Don't forgot to update the overloaded implementation........
	*/
	Maze::DIRECTION bestDir;
	float minDist = FLT_MAX;

	int maxDir = static_cast<int>(Maze::DIRECTION::TOTAL);

	for (int i = 0; i < maxDir; ++i)
	{
		Maze::DIRECTION dir = static_cast<Maze::DIRECTION>(i);

		Tile* pNextTile = GetNextTile(startPt, dir);
		if (pNextTile == nullptr) continue;
		MazePt nextPt = pNextTile->GetMazePt();

		if (nextPt.IsEqual( endPt )) return dir;

		Vector3 p1 = Transform::MazeToWorldPt( endPt );
		Vector3 p2 = pNextTile->GetWorldPt();
		float dist = (p1 - p2).Length();

		if (dist < minDist)
		{
			minDist = dist;
			bestDir = dir;
		}
	}

	return bestDir;
}

void Maze::ResetPathfindingCosts()
{
	for (Tile* pTile : m_grid)
	{
		pTile->SetGlobalCost( 9999999.f );
		pTile->SetHeuristic( 9999999.f );
		pTile->SetFinalCost( 9999999.f );
	}
}

float Maze::ManhattanDistance(const Tile* pTile1, const Tile* pTile2) const
{
	// Credits to https://www.redblobgames.com/grids/hexagons/#conversions 
	// for manhattan distance formula on hex grids

	MazePt start = pTile1->GetMazePt();
	MazePt end = pTile2->GetMazePt();

	Vector3 a;
	a.x = start.x;
	a.z = start.y - (start.x - (start.x & 1)) / 2;
	a.y = -a.x - a.z;

	Vector3 b;
	b.x = end.x;
	b.z = end.y - (end.x - (end.x & 1)) / 2;
	b.y = -b.x - b.z;

	return (Math::FAbs(a.x - b.x) + Math::FAbs(a.y - b.y) + Math::FAbs(a.z - b.z)) / 2.0f;
}

float Maze::ManhattanDistance(const MazePt& p1, const MazePt& p2) const
{
	Vector3 a;
	a.x = p1.x;
	a.z = p1.y - (p1.x - (p1.x & 1)) / 2;
	a.y = -a.x - a.z;

	Vector3 b;
	b.x = p2.x;
	b.z = p2.y - (p2.x - (p2.x & 1)) / 2;
	b.y = -b.x - b.z;

	return (Math::FAbs(a.x - b.x) + Math::FAbs(a.y - b.y) + Math::FAbs(a.z - b.z)) / 2.0f;
}




