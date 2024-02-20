#include "StatesTank.h"
#include "../ConcreteMessages.h"

#define deltaTime static_cast<float>(dt)

static PostOffice& PO = *PostOffice::GetInstance();
static SceneData& data = *SceneData::GetInstance();

#define TANK_HP 150.0f
#define TANK_RETREAT_HP 45.0f
#define TANK_RECOVERED_HP 85.0f
#define TANK_RANGE 4
#define TANK_ATK_RANGE 3
#define TANK_FOV 1
#define TANK_DMG 22.6f
#define TANK_BLAST_RADIUS 1
#define POLL_TURNS 2

#pragma region TankInit
StateTankInit::StateTankInit(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateTankInit::~StateTankInit() {}

void StateTankInit::Enter()
{
}

void StateTankInit::Update(double dt)
{
	m_entity->SetHP(TANK_HP);
	m_entity->SetMaxHP(TANK_HP);
	m_entity->SetRecoveredHP(TANK_RECOVERED_HP);

	m_entity->SetVisibilityRange(TANK_RANGE);
	m_entity->SetFOV(TANK_FOV);

	int size = data.GetNumGrid() * data.GetNumGrid();
	std::fill(m_entity->m_visited.begin(), m_entity->m_visited.end(), false);
	m_entity->m_stack.clear();
	m_entity->m_previous.resize(size);
	m_entity->m_shortestPath.clear();
	m_entity->m_visibleTiles.clear();

	m_entity->m_turnsToMove = 0;

	int randY = Math::RandIntMinMax(0, data.GetNumGrid() - 1);
	switch (m_entity->GetTeam())
	{
	case 1:
		m_entity->SetMazePt(0, 0);
		m_entity->SetDir(Maze::DIRECTION::LEFTDOWN);
		break;
	case 2:
		m_entity->SetMazePt(data.GetNumGrid() - 1, data.GetNumGrid() - 1);
		m_entity->SetDir(Maze::DIRECTION::RIGHTUP);
		break;
	}
	m_entity->SetIsDead( false );
	m_entity->SetIsActive( true );

	m_entity->GetFSM()->SetNextState("TankPatrol");
}

void StateTankInit::UpdateTurn()
{
}

void StateTankInit::Exit()
{
}
#pragma endregion

#pragma region TankPatrol
StateTankPatrol::StateTankPatrol(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateTankPatrol::~StateTankPatrol() {}

void StateTankPatrol::Enter()
{
	m_entity->SetOtherEntity( nullptr );
	m_entity->m_shortestPath.clear();
}

void StateTankPatrol::Update(double dt)
{
	// Messaged by teammate for help
	if (m_entity->GetOtherEntity() != nullptr)
	{
		m_entity->GetFSM()->SetNextState("TankChase");
		return;
	}

	MessageEnemyInLOS* msg = new MessageEnemyInLOS( m_entity );
	bool foundEnemy = PO.Send("SceneA2", msg);

	if (foundEnemy)
	{
		MazePt myPt = m_entity->GetMazePt();
		MazePt enemyPt = m_entity->GetOtherEntity()->GetMazePt();
		float distFromEnemy = data.GetWorldMaze()->ManhattanDistance(myPt, enemyPt);

		if (distFromEnemy <= TANK_ATK_RANGE)
			m_entity->GetFSM()->SetNextState("TankAttack");
		else
			m_entity->GetFSM()->SetNextState("TankChase");
	}

	if (m_entity->GetHP() < TANK_RETREAT_HP)
		m_entity->GetFSM()->SetNextState("TankRetreat");
}

void StateTankPatrol::UpdateTurn()
{
	m_entity->DFSOnce();
}

void StateTankPatrol::Exit()
{
}
#pragma endregion

#pragma region TankChase
StateTankChase::StateTankChase(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateTankChase::~StateTankChase() {}

void StateTankChase::Enter()
{
	Tile* pEnemyTile = data.GetWorldMaze()->GetTile(m_entity->GetOtherEntity()->GetMazePt());
	bool foundPath = m_entity->AStar( pEnemyTile );

	if (foundPath)
		m_enemyLastPos = pEnemyTile->GetMazePt();	// Keep track if enemy move
	else
	{
		m_entity->GetFSM()->SetNextState("TankPatrol");
		return;
	}
}

void StateTankChase::Update(double dt)
{
	if (m_entity->GetOtherEntity()->IsDead())
	{
		m_entity->GetFSM()->SetNextState("TankPatrol");
		return;
	}

	MazePt myPt = m_entity->GetMazePt();
	MazePt enemyPt = m_entity->GetOtherEntity()->GetMazePt();

	// Enemy moved, regenerate AStar
	if (enemyPt.DoesNotEqual( m_enemyLastPos ))
	{
		Tile* pEnemyTile = data.GetWorldMaze()->GetTile( enemyPt );
		bool foundPath = m_entity->AStar( pEnemyTile );

		if (foundPath)
			m_enemyLastPos = enemyPt;
		else
		{
			m_entity->GetFSM()->SetNextState("TankPatrol");
			return;
		}
	}

	m_distFromEnemy = data.GetWorldMaze()->ManhattanDistance(myPt, enemyPt);

	if (m_entity->m_shortestPath.empty())
	{
		if (m_distFromEnemy <= TANK_RANGE)
			m_entity->GetFSM()->SetNextState("TankAttack");
		else
			m_entity->GetFSM()->SetNextState("TankPatrol");

		return;
	}

	if (m_distFromEnemy <= TANK_ATK_RANGE)
		m_entity->GetFSM()->SetNextState("TankAttack");

	if (m_entity->GetHP() < TANK_RETREAT_HP)
		m_entity->GetFSM()->SetNextState("TankRetreat");
}

void StateTankChase::UpdateTurn()
{
	MazePt nextPt = m_entity->m_shortestPath.front();
	bool moved = m_entity->Move(nextPt);
	if (moved) m_entity->m_shortestPath.erase(m_entity->m_shortestPath.begin());
}

void StateTankChase::Exit()
{
}
#pragma endregion

#pragma region TankAttack
StateTankAttack::StateTankAttack(const std::string& stateID, Entity* _entity)
	: State(stateID),
	m_entity(_entity) {}

StateTankAttack::~StateTankAttack() {}

void StateTankAttack::Enter()
{
	m_pEnemy = m_entity->GetOtherEntity();

	MazePt myPt = m_entity->GetMazePt();
	MazePt enemyPt = m_pEnemy->GetMazePt();

	Maze::DIRECTION dirTowardsEnemy = data.GetWorldMaze()->GetMoveDir(myPt, enemyPt);
	m_entity->SetDir(dirTowardsEnemy);

	m_entity->m_shortestPath.clear();

	MessageCavalry* msg = new MessageCavalry(m_entity);
	PO.Send("SceneA2", msg);
}

void StateTankAttack::Update(double dt)
{
	if (m_pEnemy->IsDead())
		m_entity->GetFSM()->SetNextState("TankPatrol");

	MazePt myPt = m_entity->GetMazePt();
	MazePt enemyPt = m_pEnemy->GetMazePt();

	Maze::DIRECTION dirTowardsEnemy = data.GetWorldMaze()->GetMoveDir(myPt, enemyPt);
	m_entity->SetDir(dirTowardsEnemy);

	float distFromEnemy = data.GetWorldMaze()->ManhattanDistance(myPt, enemyPt);

	if (distFromEnemy > TANK_RANGE)
		m_entity->GetFSM()->SetNextState("TankChase");

	if (m_entity->GetHP() < TANK_RETREAT_HP)
	{
		MessageCavalry* msg = new MessageCavalry(m_entity);
		PO.Send("SceneA2", msg);

		m_entity->GetFSM()->SetNextState("TankRetreat");
	}
}

void StateTankAttack::UpdateTurn()
{
	// TODO : Spawn Grenades
	std::string enemyAddr = std::to_string(m_pEnemy->GetID());
	MessageAttack* msgATK = new MessageAttack( TANK_DMG );
	PO.Send(enemyAddr, msgATK);
}

void StateTankAttack::Exit()
{
}
#pragma endregion

#pragma region TankRetreat
StateTankRetreat::StateTankRetreat(const std::string& stateID, Entity* _entity)
	:
	State(stateID),
	m_entity(_entity) 
{}

StateTankRetreat::~StateTankRetreat() {}

void StateTankRetreat::Enter()
{
	MessageMedicHelp* msgHelp = new MessageMedicHelp(m_entity);
	PO.Send("SceneA2", msgHelp);

	Tile* pTargetTile = nullptr;
	switch (m_entity->GetTeam())
	{
		case 1: pTargetTile = data.GetWorldMaze()->GetTile(0, 0); break;
		case 2: pTargetTile = data.GetWorldMaze()->GetTile(data.GetNumGrid() - 1, data.GetNumGrid() - 1); break;
	}
	m_entity->AStar( pTargetTile );

	pollBuffer = POLL_TURNS;
}

void StateTankRetreat::Update(double dt)
{
	if (m_entity->GetHP() > TANK_RECOVERED_HP)
		m_entity->GetFSM()->SetNextState("TankPatrol");

	if (m_entity->GetHP() <= 0.0f)
		m_entity->GetFSM()->SetNextState("TankDead");
}

void StateTankRetreat::UpdateTurn()
{
	--pollBuffer;
	if (pollBuffer == 0)
	{
		MessageMedicHelp* msgHelp = new MessageMedicHelp(m_entity);
		PO.Send("SceneA2", msgHelp);

		pollBuffer = POLL_TURNS;
	}

	if (!m_entity->m_shortestPath.empty())
	{
		MazePt targetPt = m_entity->m_shortestPath.front();
		bool moved = m_entity->Move(targetPt);
		if (moved) m_entity->m_shortestPath.erase(m_entity->m_shortestPath.begin());
	}
}

void StateTankRetreat::Exit()
{
}
#pragma endregion

#pragma region TankDead
StateTankDead::StateTankDead(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateTankDead::~StateTankDead() {}

void StateTankDead::Enter()
{
	m_entity->SetIsDead( true );
}

void StateTankDead::Update(double dt)
{
	m_entity->SetHP(TANK_HP);
	m_entity->SetMaxHP(TANK_HP);
	m_entity->SetRecoveredHP(TANK_RECOVERED_HP);

	m_entity->SetVisibilityRange(TANK_RANGE);
	m_entity->SetFOV(TANK_FOV);

	int size = data.GetNumGrid() * data.GetNumGrid();
	m_entity->m_visited.resize(size, false);
	m_entity->m_stack.clear();
	m_entity->m_previous.resize(size);
	m_entity->m_shortestPath.clear();
	m_entity->m_visibleTiles.clear();

	m_entity->m_turnsToMove = 0;

	int randY = Math::RandIntMinMax(0, data.GetNumGrid() - 1);
	switch (m_entity->GetTeam())
	{
		case 1:  m_entity->SetMazePt(0, 0); break;
		case 2:  m_entity->SetMazePt(data.GetNumGrid() - 1, data.GetNumGrid() - 1); break;
	}
	m_entity->SetIsDead( true );
	m_entity->SetIsActive( false );
}

void StateTankDead::UpdateTurn()
{
}

void StateTankDead::Exit()
{
}
#pragma endregion

