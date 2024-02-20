#ifndef _CONCRETE_MESSAGES_H_
#define _CONCRETE_MESSAGES_H_

#include "Message.h"
#include "Entities/Entity.h"
#include "Maze/Tile.h"

struct MessageEnemyInLOS : public Message
{
	Entity* m_entity;

	MessageEnemyInLOS(Entity* _entity) : m_entity(_entity) {}
	virtual ~MessageEnemyInLOS() {}
};

struct MessageAttack : public Message
{
	float damage;
	
	MessageAttack(float _dmg) : damage( _dmg ) {}
	virtual ~MessageAttack() {}
};

struct MessageHeal : public Message
{
	float heal;

	MessageHeal(float _heal) : heal(_heal) {}
	virtual ~MessageHeal() {}
};

struct MessageMedicHelp : public Message
{
	Entity* pRequester;

	MessageMedicHelp(Entity* _requester) : pRequester(_requester) {}
	virtual ~MessageMedicHelp() {}
};

struct MessageCavalry : public Message
{
	Entity* pRequester;

	MessageCavalry(Entity* _pRequester) : pRequester(_pRequester) {}
	virtual ~MessageCavalry() {}
};

struct MessageIncrementKillCount : public Message
{
	Team team;

	MessageIncrementKillCount(Team _team) : team(_team) {}
	virtual ~MessageIncrementKillCount() {}
};

// TODO : Implement in Scene
struct MessageSpawnArrow : public Message
{
	Vector3 pos, dir;
	float speed, dmg;

	MessageSpawnArrow(const Vector3 &_pos,
					  const Vector3 &_dir, 
					  const float   &_speed,
					  const float   &_dmg) 
	{
		pos   = _pos;
		dir   = _dir;
		speed = _speed;
		dmg   = _dmg;
	}
	virtual ~MessageSpawnArrow() {}
};

// TODO : Implement in Scene
struct MessageSpawnGrenade : public Message
{
	Vector3 pos, dir;
	float speed, dmg, radius;

	MessageSpawnGrenade(const Vector3 &_pos,
					    const Vector3 &_dir,
					    const float   &_speed,
					    const float   &_dmg,
						const float   &_radius)
	{
		pos    = _pos;
		dir    = _dir;
		speed  = _speed;
		dmg    = _dmg;
		radius = _radius;
	}
	virtual ~MessageSpawnGrenade() {}
};

#endif
