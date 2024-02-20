#include "StatesSoldier.h"
#include "../ConcreteMessages.h"

#define deltaTime static_cast<float>(dt)

static PostOffice& PO = *PostOffice::GetInstance();
static SceneData& data = *SceneData::GetInstance();

#define SOLDIER_HP 110.0f
#define SOLDIER_RETREAT_HP 30.0f
#define SOLDIER_RECOVERED_HP 65.0f
#define SOLDIER_RANGE 2
#define SOLDIER_ATK_RANGE 1
#define SOLDIER_FOV 6
#define SOLDIER_DMG 18.5f
#define POLL_TURNS 2

#pragma region SoldierInit
StateSoldierInit::StateSoldierInit(const std::string& stateID, Entity* _entity)
	:
	State(stateID),
	m_entity(_entity)
{}

StateSoldierInit::~StateSoldierInit() {}

void StateSoldierInit::Enter()
{
}

void StateSoldierInit::Update(double dt)
{
	m_entity->SetHP(SOLDIER_HP);
	m_entity->SetMaxHP(SOLDIER_HP);
	m_entity->SetRecoveredHP(SOLDIER_RECOVERED_HP);

	m_entity->SetVisibilityRange(SOLDIER_RANGE);
	m_entity->SetFOV(SOLDIER_FOV);

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
			m_entity->SetDir( Maze::DIRECTION::LEFTDOWN );
			break;
		case 2: 
			m_entity->SetMazePt(data.GetNumGrid() - 1, data.GetNumGrid() - 1);
			m_entity->SetDir( Maze::DIRECTION::RIGHTUP );
			break;
	}
	m_entity->SetIsDead( false );
	m_entity->SetIsActive( true );

	m_entity->GetFSM()->SetNextState("SoldierPatrol");
}

void StateSoldierInit::UpdateTurn()
{
}

void StateSoldierInit::Exit()
{
}
#pragma endregion

#pragma region SoldierPatrol
StateSoldierPatrol::StateSoldierPatrol(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity)
{}

StateSoldierPatrol::~StateSoldierPatrol() {}

void StateSoldierPatrol::Enter()
{
	m_entity->SetOtherEntity( nullptr );
	m_entity->m_shortestPath.clear();
}

void StateSoldierPatrol::Update(double dt)
{	
	// Messaged by teammate for help
	if (m_entity->GetOtherEntity() != nullptr)
	{
		m_entity->GetFSM()->SetNextState("SoldierChase");
		return;
	}

	MessageEnemyInLOS* msg = new MessageEnemyInLOS( m_entity );
	bool foundEnemy = PO.Send("SceneA2", msg);

	if (foundEnemy)
	{
		MazePt myPt = m_entity->GetMazePt();
		MazePt enemyPt = m_entity->GetOtherEntity()->GetMazePt();
		float distFromEnemy = data.GetWorldMaze()->ManhattanDistance(myPt, enemyPt);

		if (distFromEnemy <= SOLDIER_ATK_RANGE)
			m_entity->GetFSM()->SetNextState("SoldierAttack");
		else
			m_entity->GetFSM()->SetNextState("SoldierChase");
	}

	if (m_entity->GetHP() < SOLDIER_RETREAT_HP)
		m_entity->GetFSM()->SetNextState("SoldierRetreat");
}

void StateSoldierPatrol::UpdateTurn()
{
	m_entity->DFSOnce();
}

void StateSoldierPatrol::Exit()
{
}
#pragma endregion

#pragma region SoldierChase
StateSoldierChase::StateSoldierChase(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateSoldierChase::~StateSoldierChase() {}

void StateSoldierChase::Enter()
{
	Tile* pEnemyTile = data.GetWorldMaze()->GetTile( m_entity->GetOtherEntity()->GetMazePt() );
	bool foundPath = m_entity->AStar( pEnemyTile );

	if (foundPath)
		m_enemyLastPos = pEnemyTile->GetMazePt();	// Keep track if enemy move
	else
	{
		m_entity->GetFSM()->SetNextState("SoldierPatrol");
		return;
	}
}

void StateSoldierChase::Update(double dt)
{
	if (m_entity->GetOtherEntity()->IsDead())
	{
		m_entity->GetFSM()->SetNextState("SoldierPatrol");
		return;
	}

	MazePt myPt = m_entity->GetMazePt();
	MazePt enemyPt = m_entity->GetOtherEntity()->GetMazePt();

	// Enemy moved, regenerate AStar
	if (enemyPt.DoesNotEqual( m_enemyLastPos) )
	{
		Tile* pEnemyTile = data.GetWorldMaze()->GetTile( enemyPt );
		bool foundPath = m_entity->AStar( pEnemyTile );

		if (foundPath)
			m_enemyLastPos = enemyPt;
		else
		{
			m_entity->GetFSM()->SetNextState("SoldierPatrol");
			return;
		}
	}

	m_distFromEnemy = data.GetWorldMaze()->ManhattanDistance( myPt, enemyPt );

	if (m_entity->m_shortestPath.empty())
	{
		if (m_distFromEnemy <= SOLDIER_RANGE)
			m_entity->GetFSM()->SetNextState("SoldierAttack");
		else
			m_entity->GetFSM()->SetNextState("SoldierPatrol");

		return;
	}

	if (m_distFromEnemy <= SOLDIER_ATK_RANGE)
		m_entity->GetFSM()->SetNextState("SoldierAttack");

	if (m_entity->GetHP() < SOLDIER_RETREAT_HP)
	{
		m_entity->GetFSM()->SetNextState("SoldierRetreat");
	}
}

void StateSoldierChase::UpdateTurn()
{
	MazePt nextPt = m_entity->m_shortestPath.front();
	bool moved = m_entity->Move( nextPt );
	if (moved) m_entity->m_shortestPath.erase(m_entity->m_shortestPath.begin());
}

void StateSoldierChase::Exit()
{
}
#pragma endregion

#pragma region SoldierAttack
StateSoldierAttack::StateSoldierAttack(const std::string& stateID, Entity* _entity) 
	:
	State(stateID),
	m_entity(_entity) 
{}

StateSoldierAttack::~StateSoldierAttack() {}

void StateSoldierAttack::Enter()
{
	m_pEnemy = m_entity->GetOtherEntity();

	MazePt myPt = m_entity->GetMazePt();
	MazePt enemyPt = m_pEnemy->GetMazePt();

	Maze::DIRECTION dirTowardsEnemy = data.GetWorldMaze()->GetMoveDir( myPt, enemyPt );
	m_entity->SetDir( dirTowardsEnemy );

	m_entity->m_shortestPath.clear();

	MessageCavalry* msg = new MessageCavalry(m_entity);
	PO.Send("SceneA2", msg);
}

void StateSoldierAttack::Update(double dt)
{
	if (m_pEnemy->IsDead())
		m_entity->GetFSM()->SetNextState("SoldierPatrol");

	MazePt myPt = m_entity->GetMazePt();
	MazePt enemyPt = m_pEnemy->GetMazePt();

	Maze::DIRECTION dirTowardsEnemy = data.GetWorldMaze()->GetMoveDir(myPt, enemyPt);
	m_entity->SetDir(dirTowardsEnemy);

	float distFromEnemy = data.GetWorldMaze()->ManhattanDistance(myPt, enemyPt);

	if (distFromEnemy > SOLDIER_RANGE)
		m_entity->GetFSM()->SetNextState("SoldierChase");

	if (m_entity->GetHP() < SOLDIER_RETREAT_HP)
	{
		MessageCavalry* msg = new MessageCavalry(m_entity);
		PO.Send("SceneA2", msg);

		m_entity->GetFSM()->SetNextState("SoldierRetreat");
	}
}

void StateSoldierAttack::UpdateTurn()
{
	std::string enemyAddr = std::to_string(m_pEnemy->GetID());
	MessageAttack* msgATK = new MessageAttack( SOLDIER_DMG );
	PO.Send(enemyAddr, msgATK);
}

void StateSoldierAttack::Exit()
{
}
#pragma endregion

#pragma region SoldierRetreat
StateSoldierRetreat::StateSoldierRetreat(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateSoldierRetreat::~StateSoldierRetreat() {}

void StateSoldierRetreat::Enter()
{
	MessageMedicHelp* msgHelp = new MessageMedicHelp( m_entity ); 
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

void StateSoldierRetreat::Update(double dt)
{
	if (m_entity->GetHP() > SOLDIER_RECOVERED_HP)
		m_entity->GetFSM()->SetNextState("SoldierPatrol");

	if (m_entity->GetHP() <= 0.0f)
		m_entity->GetFSM()->SetNextState("SoldierDead");
}

void StateSoldierRetreat::UpdateTurn()
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
		bool moved = m_entity->Move( targetPt );
		if (moved) m_entity->m_shortestPath.erase(m_entity->m_shortestPath.begin());
	}
}

void StateSoldierRetreat::Exit()
{
}
#pragma endregion

#pragma region SoldierDead
StateSoldierDead::StateSoldierDead(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateSoldierDead::~StateSoldierDead() {}

void StateSoldierDead::Enter()
{
	m_entity->SetIsDead( true );
}

void StateSoldierDead::Update(double dt)
{
	m_entity->SetHP(SOLDIER_HP);
	m_entity->SetMaxHP(SOLDIER_HP);
	m_entity->SetRecoveredHP(SOLDIER_RECOVERED_HP);

	m_entity->SetVisibilityRange(SOLDIER_RANGE);
	m_entity->SetFOV(SOLDIER_FOV);

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
		case 1:  m_entity->SetMazePt(0, 0); break;
		case 2:  m_entity->SetMazePt(data.GetNumGrid() - 1, data.GetNumGrid() - 1); break;
	}
	m_entity->SetIsDead( true );
	m_entity->SetIsActive( false );
}

void StateSoldierDead::UpdateTurn()
{
}

void StateSoldierDead::Exit()
{
}
#pragma endregion

