#ifndef _SCENE_A2_H_
#define _SCENE_A2_H_

// Headers
#include "SceneBase.h"
#include "ConcreteMessages.h"
#include "Entities/Entity.h"

// STL
#include <vector>
#include <queue>
#include <set>
#include <fstream>
#include <sstream>

class SceneA2 : public SceneBase, public ObjectBase
{
public:
	static const int WIN_SCORE = 12;
	static const int MAX_TURNS = 315;
	const float		 EVENT_CHANCE = 86.f;	// From 0 - 100
public:
	enum class UPDATE_STATE
	{
		ENTER,
		MOVE,
		PAUSE,
		SWITCH_TURN,
	};
	enum class EVENT
	{
		METEOR,
		DEATH,
		SPAWN,

		NUM_TOTAL,
	};
	struct MazeParameters
	{
		int	   m_numOctaves;
		double m_persistance;

		MazeParameters(int _octaves, double _persistance)
			:
			m_numOctaves( _octaves ),
			m_persistance( _persistance )
		{}
	};
public:
	SceneA2();
	~SceneA2();

	void Init();
	void Update(double dt);
	void UpdateTurn(const Team teamNumber);
	void Render();
	void Exit();

	void RenderTile(Tile *tile, bool tileIsVisible);
	Entity* FetchEntity(Entity::TYPE _type, Team _team);
	void RenderEntity(Entity* entity, float zOffset);
	void RenderEntityHP(Entity* entity, float zOffset);
	void RenderEntityState(Entity* entity);

	bool Handle(Message* msg);
	void ResetGame();
	std::set<Tile*> GetVisibleTiles(const Team _team);
	std::vector<Tile*> TilesWithinRange(const MazePt& mazePt, int radius);
private:
	std::vector<Entity*> m_EntityList;

	Maze* m_maze;
	std::vector<MazeParameters> m_mazeParams;
	int m_currMazeIdx;
	std::vector<Tile*> m_vecVisibleTiles;
	std::set<Tile*> m_setVisibleTiles;

	UPDATE_STATE m_updateState;

	bool  m_GameIsOver;
	int	  m_scoreTeam1;
	int   m_scoreTeam2;
	int	  m_numTeam1;
	int	  m_numTeam2;
	Team  m_winner;

	float m_timeTillNextState;
	float m_timeTillNextEvent;

	float m_speed;
	float m_bounceTime;
	int   m_numTurn;
	Team  m_currTeam;
};

#endif