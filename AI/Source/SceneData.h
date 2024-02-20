#ifndef _SCENE_DATA_
#define _SCENE_DATA_

#include "Vector3.h"
#include "../../Common/Source/SingletonTemplate.h"
#include "Maze/Maze.h"

class SceneData : public Singleton<SceneData>
{
	friend Singleton<SceneData>;
public:
	Vector3 GetWorldSize() const;
	float GetWorldWidth() const;
	float GetWorldHeight() const;
	void SetWorldSize(const Vector3 &_worldSize);
	void SetWorldSize(int _worldWidth, int _worldHeight);

	int GetNumGrid() const;
	void SetNumGrid(int _numGrid);

	float GetGridOffset() const;
	void  SetGridOffset(float _gridOffset);

	float GetGridSize() const;
	void SetGridSize(float _gridSize);

	Maze* GetWorldMaze() const;
	void SetWorldMaze(Maze* _pMaze);
private:
	Vector3 m_worldSize; 
	int	    m_noGrid;
	float   m_gridSize;
	float   m_gridOffset;
	Maze*	m_maze;
private:
	SceneData();
	SceneData(const SceneData&) = delete;
	~SceneData();
};

namespace Helper
{
	Vector3 GetMousePos();
	MazePt GetMouseGrid2D();
	
}

namespace Transform
{
	Vector3 MazeToWorldPt(const MazePt& mazePt);
	int Grid2Dto1D(const MazePt &mazePt);
	int Grid2Dto1D(int x, int y);
}

#endif
