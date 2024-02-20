#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "Vector3.h"
#include "../States/StateMachine.h"
#include "../SceneData.h"
#include "../Maze/Maze.h"

#include "../ObjectBase.h"
#include "../Message.h"
#include "../PostOffice.h"

using Team = int;

class Entity : public ObjectBase
{
public:
	enum class TYPE
	{
		SOLDIER, 
		ARCHER, ARROW,
		MEDIC,
		TANK, GRENADE,

		EXPLOSION,

		TOTAL,
	};
public:
	Entity(TYPE _type, Team _team);
	~Entity();

	void Init();

	void DFSOnce();
	bool AStar(Tile* pTargetTile);

	bool Handle(Message* msg); 
	bool IsSameTeamAs(const Entity* other) const;
	bool Move(const Maze::DIRECTION _dir);
	bool Move(const MazePt& _targetPt);
	std::vector<Tile*> CalcVisibleTiles();

	// Getter/Setters
	int GetID() const;

	TYPE GetType() const;
	Team GetTeam() const;

	bool IsActive() const;
	void SetIsActive(bool _isActive);

	bool IsDead() const;
	void SetIsDead(bool _isDead);

	MazePt GetMazePt() const;
	void SetMazePt(const MazePt& _mazePt);
	void SetMazePt(int _x, int _y);

	float GetHP() const;
	void SetHP(int _HP);
	float GetMaxHP() const;
	void SetMaxHP(int _HP);
	float GetRecoveredHP() const;
	void SetRecoveredHP(int _recoveredHP);

	Entity* GetOtherEntity() const;
	void SetOtherEntity(Entity* _entity);

	Maze::DIRECTION GetDir() const;
	void SetDir(const Maze::DIRECTION _dir);

	int GetVisibilityRange() const;
	void SetVisibilityRange(int _visRange);
	int GetFOV() const;
	void SetFOV(int _degrees);

	StateMachine* GetFSM() const;
	void AssignStateMachine();

	// Helper
	void PrintDir(const Maze::DIRECTION dir) const;
	// bool CompareTileCost();
public:
	std::vector<bool>	m_visited;
	std::vector<MazePt> m_stack;
	std::vector<MazePt> m_previous;
	std::vector<MazePt> m_shortestPath;
	std::vector<Tile*>  m_visibleTiles;

	int   m_turnsToMove;
	float m_lifeTime;
protected:
	const int  m_ID;
	const TYPE m_type;
	const Team m_team;

	bool    m_isActive;
	bool	m_isDead;
	float   m_HP;
	float   m_MaxHP;
	float	m_recoveredHP;
	Entity*	m_other;

	MazePt			m_currPt;
	Maze::DIRECTION m_dir;
	int				m_visibleRange;
	int				m_FOV;	// Measured in blocks (deviation from front dir)
	
	std::vector<Maze::DIRECTION> m_dirList;

	StateMachine* m_stateMachine;
private:
	static int m_nextValidID;
};

#endif