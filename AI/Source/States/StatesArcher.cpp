#include "StatesArcher.h"
#include "../ConcreteMessages.h"

#define deltaTime static_cast<float>(dt)

static PostOffice& PO = *PostOffice::GetInstance();
static SceneData& data = *SceneData::GetInstance();

#define ARCHER_HP 100.0f
#define ARCHER_RETREAT_HP 20.0f
#define ARCHER_RECOVERED_HP 50.0f
#define ARCHER_RANGE 5
#define ARCHER_ATK_RANGE 4
#define ARCHER_FOV 2
#define ARCHER_DMG 12.85f
#define POLL_TURNS 3

#pragma region ArcherInit
StateArcherInit::StateArcherInit(const std::string& stateID, Entity* _entity)
	:
	State(stateID),
	m_entity(_entity)
{}

StateArcherInit::~StateArcherInit() {}

void StateArcherInit::Enter()
{
}

void StateArcherInit::Update(double dt)
{
	m_entity->SetHP(ARCHER_HP);
	m_entity->SetMaxHP(ARCHER_HP);
	m_entity->SetRecoveredHP(ARCHER_RECOVERED_HP);

	m_entity->SetVisibilityRange(ARCHER_RANGE);
	m_entity->SetFOV(ARCHER_FOV);

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

	m_entity->GetFSM()->SetNextState("ArcherPatrol");
}

void StateArcherInit::UpdateTurn()
{
}

void StateArcherInit::Exit()
{
}
#pragma endregion

#pragma region ArcherPatrol
StateArcherPatrol::StateArcherPatrol(const std::string& stateID, Entity* _entity)
	:
	State(stateID),
	m_entity(_entity)
{}

StateArcherPatrol::~StateArcherPatrol() {}

void StateArcherPatrol::Enter()
{
	m_entity->SetOtherEntity( nullptr );
	m_entity->m_shortestPath.clear();
}

void StateArcherPatrol::Update(double dt)
{
	// Messaged by teammate for help
	if (m_entity->GetOtherEntity() != nullptr)
	{
		m_entity->GetFSM()->SetNextState("ArcherChase");
		return;
	}

	MessageEnemyInLOS* msg = new MessageEnemyInLOS( m_entity );
	bool foundEnemy = PO.Send("SceneA2", msg);

	if (foundEnemy)
	{
		MazePt myPt = m_entity->GetMazePt();
		MazePt enemyPt = m_entity->GetOtherEntity()->GetMazePt();
		float distFromEnemy = data.GetWorldMaze()->ManhattanDistance(myPt, enemyPt);

		if (distFromEnemy <= ARCHER_ATK_RANGE)
			m_entity->GetFSM()->SetNextState("ArcherAttack");
		else
			m_entity->GetFSM()->SetNextState("ArcherChase");
	}

	if (m_entity->GetHP() < ARCHER_RETREAT_HP)
		m_entity->GetFSM()->SetNextState("ArcherRetreat");
}

void StateArcherPatrol::UpdateTurn()
{
	m_entity->DFSOnce();
}

void StateArcherPatrol::Exit()
{
}
#pragma endregion

#pragma region ArcherChase
StateArcherChase::StateArcherChase(const std::string& stateID, Entity* _entity)
	:
	State(stateID),
	m_entity(_entity)
{}

StateArcherChase::~StateArcherChase() {}

void StateArcherChase::Enter()
{
	Tile* pEnemyTile = data.GetWorldMaze()->GetTile(m_entity->GetOtherEntity()->GetMazePt());
	bool foundPath = m_entity->AStar(pEnemyTile);

	if (foundPath)
		m_enemyLastPos = pEnemyTile->GetMazePt();	// To keep track of if the enemy moved
	else
	{
		m_entity->GetFSM()->SetNextState("ArcherPatrol");
		return;
	}
}

void StateArcherChase::Update(double dt)
{
	if (m_entity->GetOtherEntity()->IsDead())
	{
		m_entity->GetFSM()->SetNextState("ArcherPatrol");
		return;
	}

	MazePt myPt = m_entity->GetMazePt();
	MazePt enemyPt = m_entity->GetOtherEntity()->GetMazePt();

	// Enemy moved, regenerate AStar path
	if (enemyPt.DoesNotEqual( m_enemyLastPos ))
	{
		Tile* pEnemyTile = data.GetWorldMaze()->GetTile(enemyPt);
		bool foundPath = m_entity->AStar(pEnemyTile);

		if (foundPath)
			m_enemyLastPos = enemyPt;
		else
		{
			m_entity->GetFSM()->SetNextState("ArcherPatrol");
			return;
		}
	}

	m_distFromEnemy = data.GetWorldMaze()->ManhattanDistance(myPt, enemyPt);

	if (m_entity->m_shortestPath.empty())
	{
		if (m_distFromEnemy <= ARCHER_RANGE)
			m_entity->GetFSM()->SetNextState("ArcherAttack");
		else
			m_entity->GetFSM()->SetNextState("ArcherPatrol");

		return;
	}

	if (m_distFromEnemy <= ARCHER_ATK_RANGE)
		m_entity->GetFSM()->SetNextState("ArcherAttack");

	if (m_entity->GetHP() < ARCHER_RETREAT_HP)
		m_entity->GetFSM()->SetNextState("ArcherRetreat");
}

void StateArcherChase::UpdateTurn()
{
	MazePt nextPt = m_entity->m_shortestPath.front();
	bool moved = m_entity->Move(nextPt);
	if (moved) m_entity->m_shortestPath.erase(m_entity->m_shortestPath.begin());
}

void StateArcherChase::Exit()
{
}
#pragma endregion

#pragma region ArcherAttack
StateArcherAttack::StateArcherAttack(const std::string& stateID, Entity* _entity)
	:
	State(stateID),
	m_entity(_entity)
{}

StateArcherAttack::~StateArcherAttack() {}

void StateArcherAttack::Enter()
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

void StateArcherAttack::Update(double dt)
{
	if (m_pEnemy->IsDead())
		m_entity->GetFSM()->SetNextState("ArcherPatrol");

	MazePt myPt = m_entity->GetMazePt();
	MazePt enemyPt = m_pEnemy->GetMazePt();

	Maze::DIRECTION dirTowardsEnemy = data.GetWorldMaze()->GetMoveDir(myPt, enemyPt);
	m_entity->SetDir(dirTowardsEnemy);

	float distFromEnemy = data.GetWorldMaze()->ManhattanDistance(myPt, enemyPt);

	if (distFromEnemy > ARCHER_RANGE)
		m_entity->GetFSM()->SetNextState("ArcherChase");

	if (m_entity->GetHP() < ARCHER_RETREAT_HP)
	{
		MessageCavalry* msg = new MessageCavalry(m_entity);
		PO.Send("SceneA2", msg);

		m_entity->GetFSM()->SetNextState("ArcherRetreat");
	}
}

void StateArcherAttack::UpdateTurn()
{
	std::string enemyAddr = std::to_string(m_pEnemy->GetID());
	MessageAttack* msgATK = new MessageAttack( ARCHER_DMG );
	PO.Send(enemyAddr, msgATK);
}

void StateArcherAttack::Exit()
{
}
#pragma endregion

#pragma region ArcherRetreat
StateArcherRetreat::StateArcherRetreat(const std::string& stateID, Entity* _entity)
	:
	State(stateID),
	m_entity(_entity)
{}

StateArcherRetreat::~StateArcherRetreat() {}

void StateArcherRetreat::Enter()
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

void StateArcherRetreat::Update(double dt)
{
	if (m_entity->GetHP() > ARCHER_RECOVERED_HP)
		m_entity->GetFSM()->SetNextState("ArcherPatrol");

	if (m_entity->GetHP() <= 0.0f)
		m_entity->GetFSM()->SetNextState("ArcherDead");
}

void StateArcherRetreat::UpdateTurn()
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

void StateArcherRetreat::Exit()
{
}
#pragma endregion

#pragma region ArcherDead
StateArcherDead::StateArcherDead(const std::string& stateID, Entity* _entity)
	:
	State(stateID),
	m_entity(_entity)
{}

StateArcherDead::~StateArcherDead() {}

void StateArcherDead::Enter()
{
	m_entity->SetIsDead( true );
}

void StateArcherDead::Update(double dt)
{
	m_entity->SetHP(ARCHER_HP);
	m_entity->SetMaxHP(ARCHER_HP);
	m_entity->SetRecoveredHP(ARCHER_RECOVERED_HP);

	m_entity->SetVisibilityRange(ARCHER_RANGE);
	m_entity->SetFOV(ARCHER_FOV);

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

void StateArcherDead::UpdateTurn()
{
}

void StateArcherDead::Exit()
{
}
#pragma endregion
