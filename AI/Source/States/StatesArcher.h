#ifndef _STATES_ARCHER_H_
#define _STATES_ARCHER_H_

#include "../Entities/Entity.h"

class StateArcherInit : public State
{
private:
	Entity* m_entity;
public:
	StateArcherInit(const std::string& stateID, Entity* go);
	virtual ~StateArcherInit();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateArcherPatrol : public State
{
private:
	Entity* m_entity;
public:
	StateArcherPatrol(const std::string& stateID, Entity* go);
	virtual ~StateArcherPatrol();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateArcherChase : public State
{
private:
	Entity* m_entity;
	float m_distFromEnemy;
	MazePt m_enemyLastPos;
public:
	StateArcherChase(const std::string& stateID, Entity* go);
	virtual ~StateArcherChase();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateArcherAttack : public State
{
private:
	Entity* m_entity;
	Entity* m_pEnemy;
public:
	StateArcherAttack(const std::string& stateID, Entity* go);
	virtual ~StateArcherAttack();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateArcherRetreat : public State
{
private:
	Entity* m_entity;
	int pollBuffer;
public:
	StateArcherRetreat(const std::string& stateID, Entity* go);
	virtual ~StateArcherRetreat();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateArcherDead : public State
{
private:
	Entity* m_entity;
public:
	StateArcherDead(const std::string& stateID, Entity* go);
	virtual ~StateArcherDead();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

#endif