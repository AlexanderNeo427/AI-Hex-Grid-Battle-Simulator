#ifndef _STATES_SOLDIER_H_
#define _STATES_SOLDIER_H_

#include "../Entities/Entity.h"

class StateSoldierInit : public State
{
private:
	Entity* m_entity;
public:
	StateSoldierInit(const std::string& stateID, Entity* go);
	virtual ~StateSoldierInit();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateSoldierPatrol : public State
{
private:
	Entity* m_entity;
public:
	StateSoldierPatrol(const std::string& stateID, Entity* go);
	virtual ~StateSoldierPatrol();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateSoldierChase : public State
{
private:
	Entity* m_entity;
	float m_distFromEnemy;
	MazePt m_enemyLastPos;
public:
	StateSoldierChase(const std::string& stateID, Entity* go);
	virtual ~StateSoldierChase();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateSoldierAttack : public State
{
	Entity* m_entity;
	Entity* m_pEnemy;
public:
	StateSoldierAttack(const std::string& stateID, Entity* go);
	virtual ~StateSoldierAttack();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateSoldierRetreat : public State
{
	Entity* m_entity;
	int pollBuffer;
public:
	StateSoldierRetreat(const std::string& stateID, Entity* go);
	virtual ~StateSoldierRetreat();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateSoldierDead : public State
{
	Entity* m_entity;
public:
	StateSoldierDead(const std::string& stateID, Entity* go);
	virtual ~StateSoldierDead();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

#endif