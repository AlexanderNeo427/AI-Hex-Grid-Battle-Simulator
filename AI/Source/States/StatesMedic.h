#ifndef _STATES_MEDIC_H_
#define _STATES_MEDIC_H_

#include "../Entities/Entity.h"

class StateMedicInit : public State
{
private:
	Entity* m_entity;
public:
	StateMedicInit(const std::string& stateID, Entity* go);
	virtual ~StateMedicInit();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateMedicPatrol : public State
{
private:
	Entity* m_entity;
public:
	StateMedicPatrol(const std::string& stateID, Entity* go);
	virtual ~StateMedicPatrol();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateMedicChase : public State
{
private:
	Entity* m_entity;
	float m_distFromTeammate;
	MazePt m_teammateLastPos;
public:
	StateMedicChase(const std::string& stateID, Entity* go);
	virtual ~StateMedicChase();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateMedicHeal : public State
{
	Entity* m_entity;
	Entity* m_pTeammate;
public:
	StateMedicHeal(const std::string& stateID, Entity* go);
	virtual ~StateMedicHeal();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateMedicRetreat : public State
{
	Entity* m_entity;
	int pollBuffer;
public:
	StateMedicRetreat(const std::string& stateID, Entity* go);
	virtual ~StateMedicRetreat();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

class StateMedicDead : public State
{
	Entity* m_entity;
public:
	StateMedicDead(const std::string& stateID, Entity* go);
	virtual ~StateMedicDead();

	virtual void Enter();
	virtual void Update(double dt);
	virtual void UpdateTurn();
	virtual void Exit();
};

#endif