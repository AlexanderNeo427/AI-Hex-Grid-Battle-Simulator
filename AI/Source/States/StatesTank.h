#ifndef _STATES_TANK_H_
#define _STATES_TANK_H_

#include "../Entities/Entity.h"

class StateTankInit : public State
{
private:
	Entity* m_entity;
public:
	StateTankInit(const std::string& stateID, Entity* go);
	virtual ~StateTankInit();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateTankPatrol : public State
{
private:
	Entity* m_entity;
public:
	StateTankPatrol(const std::string& stateID, Entity* go);
	virtual ~StateTankPatrol();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateTankChase : public State
{
private:
	Entity* m_entity;
	float m_distFromEnemy;
	MazePt m_enemyLastPos;
public:
	StateTankChase(const std::string& stateID, Entity* go);
	virtual ~StateTankChase();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateTankAttack : public State
{
private:
	Entity* m_entity;
	Entity* m_pEnemy;
public:
	StateTankAttack(const std::string& stateID, Entity* go);
	virtual ~StateTankAttack();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateTankRetreat : public State
{
private:
	Entity* m_entity;
	int pollBuffer;
public:
	StateTankRetreat(const std::string& stateID, Entity* go);
	virtual ~StateTankRetreat();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateTankDead : public State
{
private:
	Entity* m_entity;
public:
	StateTankDead(const std::string& stateID, Entity* go);
	virtual ~StateTankDead();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

#endif