//#ifndef SCENE_HEX_H
//#define SCENE_HEX_H
//
//#include "GameObject.h"
//#include <vector>
//#include "SceneBase.h"
//#include "Maze.h"
//#include <queue>
//
//class SceneHex : public SceneBase
//{
//public:
//	SceneHex();
//	~SceneHex();
//
//	virtual void Init();
//	virtual void Update(double dt);
//	virtual void Render();
//	virtual void Exit();
//
//	void RenderGO(GameObject *go);
//	GameObject* FetchGO();
//
//	void DFS(MazePt curr);
//	bool BFS(MazePt start, MazePt end);
//
//protected:
//
//	std::vector<GameObject *> m_goList;
//	float m_speed;
//	float m_worldWidth;
//	float m_worldHeight;
//	int m_objectCount;
//	int m_noGrid;
//	float m_gridSize;
//	float m_gridOffset;
//
//	Maze m_maze;
//	MazePt m_start;
//	MazePt m_end;
//	std::vector<Maze::TILE_CONTENT> m_myGrid; //read maze and store here
//	std::vector<bool> m_visited; //visited set for DFS/BFS
//	std::queue<MazePt> m_queue; //queue for BFS
//	std::vector<MazePt> m_previous; //to store previous tile
//	std::vector<MazePt> m_shortestPath;  //to store shortest path
//	unsigned m_mazeKey;
//	float m_wallLoad;
//};
//
//#endif