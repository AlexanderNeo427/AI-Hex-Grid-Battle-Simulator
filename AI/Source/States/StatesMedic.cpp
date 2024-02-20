#include "StatesMedic.h"
#include "../ConcreteMessages.h"

#define deltaTime static_cast<float>(dt)

static PostOffice& PO = *PostOffice::GetInstance();
static SceneData& data = *SceneData::GetInstance();

#define MEDIC_HP 85.0f
#define MEDIC_RETREAT_HP 30.0f
#define MEDIC_RECOVERED_HP 55.0f
#define MEDIC_RANGE 2
#define MEDIC_HEAL_RANGE 2
#define MEDIC_FOV 2
#define MEDIC_HEAL_PWR 7.5f
#define POLL_TURNS 2

#pragma region MedicInit
StateMedicInit::StateMedicInit(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateMedicInit::~StateMedicInit() {}

void StateMedicInit::Enter()
{
}

void StateMedicInit::Update(double dt)
{
	m_entity->SetHP(MEDIC_HP);
	m_entity->SetMaxHP(MEDIC_HP);
	m_entity->SetRecoveredHP(MEDIC_RECOVERED_HP);

	m_entity->SetVisibilityRange(MEDIC_RANGE);
	m_entity->SetFOV(MEDIC_FOV);

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

	m_entity->GetFSM()->SetNextState("MedicPatrol");
}

void StateMedicInit::UpdateTurn()
{
}

void StateMedicInit::Exit()
{
}
#pragma endregion

#pragma region MedicPatrol
StateMedicPatrol::StateMedicPatrol(const std::string& stateID, Entity* _entity)
	:
	State(stateID),
	m_entity(_entity)
{}

StateMedicPatrol::~StateMedicPatrol() {}

void StateMedicPatrol::Enter()
{
	m_entity->SetOtherEntity( nullptr );
	m_entity->m_shortestPath.clear();
}

void StateMedicPatrol::Update(double dt)
{
	// Received message for help from teammates
	if (m_entity->GetOtherEntity() != nullptr)
	{
		if (m_entity->GetOtherEntity()->IsActive() &&
			!m_entity->GetOtherEntity()->IsDead())
		{
			MazePt myPt = m_entity->GetMazePt();
			MazePt teammatePt = m_entity->GetOtherEntity()->GetMazePt();
			float dist = data.GetWorldMaze()->ManhattanDistance( myPt, teammatePt );

			if (dist <= MEDIC_HEAL_RANGE)
			{
				m_entity->GetFSM()->SetNextState("MedicHeal");
				return;
			}
			else
			{
				m_entity->GetFSM()->SetNextState("MedicChase");
				return;
			}
		}
		else
		{
			m_entity->SetOtherEntity( nullptr );
		}
	}

	if (m_entity->GetHP() < MEDIC_RETREAT_HP)
	{
		m_entity->GetFSM()->SetNextState("MedicRetreat");
		return;
	}
}

void StateMedicPatrol::UpdateTurn()
{
	m_entity->DFSOnce();
}

void StateMedicPatrol::Exit()
{
}
#pragma endregion

#pragma region MedicChase
StateMedicChase::StateMedicChase(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateMedicChase::~StateMedicChase() {}

void StateMedicChase::Enter()
{
	Tile* pTeammateTile = data.GetWorldMaze()->GetTile(m_entity->GetOtherEntity()->GetMazePt());
	bool foundPath = m_entity->AStar( pTeammateTile );

	if (foundPath)
		m_teammateLastPos = pTeammateTile->GetMazePt();	// Keep track of if teammate moved
	else
	{
		m_entity->GetFSM()->SetNextState("MedicPatrol");
		return;
	}
}

void StateMedicChase::Update(double dt)
{
	if (m_entity->GetOtherEntity()->IsDead())
	{
		m_entity->GetFSM()->SetNextState("MedicPatrol");
		return;
	}

	MazePt myPt = m_entity->GetMazePt();
	MazePt teammatePt = m_entity->GetOtherEntity()->GetMazePt();

	// Teammate moved, regenerate AStar
	if (teammatePt.DoesNotEqual( m_teammateLastPos ))
	{
		Tile* pTeammateTile = data.GetWorldMaze()->GetTile( teammatePt );
		bool foundPath = m_entity->AStar( pTeammateTile );

		if (foundPath)
			m_teammateLastPos = teammatePt;
		else
		{
			m_entity->GetFSM()->SetNextState("SoldierPatrol");
			return;
		}
	}

	m_distFromTeammate = data.GetWorldMaze()->ManhattanDistance(myPt, teammatePt);

	if (m_entity->m_shortestPath.empty())
	{
		if (m_distFromTeammate <= MEDIC_RANGE)
			m_entity->GetFSM()->SetNextState("MedicHeal");
		else
			m_entity->GetFSM()->SetNextState("MedicPatrol");

		return;
	}

	if (m_distFromTeammate <= MEDIC_HEAL_RANGE)
		m_entity->GetFSM()->SetNextState("MedicHeal");

	if (m_entity->GetHP() < MEDIC_RETREAT_HP)
		m_entity->GetFSM()->SetNextState("SoldierRetreat");
}

void StateMedicChase::UpdateTurn()
{
	MazePt nextPt = m_entity->m_shortestPath.front();
	bool moved = m_entity->Move( nextPt );
	if (moved) m_entity->m_shortestPath.erase(m_entity->m_shortestPath.begin());
}

void StateMedicChase::Exit()
{
}
#pragma endregion

#pragma region MedicHeal
StateMedicHeal::StateMedicHeal(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateMedicHeal::~StateMedicHeal() {}

void StateMedicHeal::Enter()
{
	m_pTeammate = m_entity->GetOtherEntity();

	MazePt myPt = m_entity->GetMazePt();
	MazePt teammatePt = m_pTeammate->GetMazePt();

	Maze::DIRECTION dirTowardsTeammate = data.GetWorldMaze()->GetMoveDir(myPt, teammatePt);
	m_entity->SetDir(dirTowardsTeammate);

	m_entity->m_shortestPath.clear();
}

void StateMedicHeal::Update(double dt)
{
	if (m_pTeammate->IsDead())
		m_entity->GetFSM()->SetNextState("MedicPatrol");

	if (m_pTeammate->GetHP() > m_pTeammate->GetRecoveredHP())
		m_entity->GetFSM()->SetNextState("MedicPatrol");

	MazePt myPt = m_entity->GetMazePt();
	MazePt teammatePt = m_pTeammate->GetMazePt();

	Maze::DIRECTION dirTowardsTeammate = data.GetWorldMaze()->GetMoveDir(myPt, teammatePt);
	m_entity->SetDir(dirTowardsTeammate);

	float distFromTeammate = data.GetWorldMaze()->ManhattanDistance(myPt, teammatePt);

	if (distFromTeammate > MEDIC_RANGE)
		m_entity->GetFSM()->SetNextState("MedicChase");

	if (m_entity->GetHP() < MEDIC_RETREAT_HP)
		m_entity->GetFSM()->SetNextState("MedicRetreat");
}

void StateMedicHeal::UpdateTurn()
{
	std::string teammateAddr = std::to_string(m_pTeammate->GetID());
	MessageHeal* msgHeal = new MessageHeal( MEDIC_HEAL_PWR );
	PO.Send(teammateAddr, msgHeal);
}

void StateMedicHeal::Exit()
{
}
#pragma endregion

#pragma region MedicRetreat
StateMedicRetreat::StateMedicRetreat(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity) 
{}

StateMedicRetreat::~StateMedicRetreat() {}

void StateMedicRetreat::Enter()
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

void StateMedicRetreat::Update(double dt)
{
	if (m_entity->GetHP() > MEDIC_RECOVERED_HP)
		m_entity->GetFSM()->SetNextState("MedicPatrol");

	if (m_entity->GetHP() <= 0.0f)
		m_entity->GetFSM()->SetNextState("MedicDead");
}

void StateMedicRetreat::UpdateTurn()
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

void StateMedicRetreat::Exit()
{
}
#pragma endregion

#pragma region MedicDead
StateMedicDead::StateMedicDead(const std::string& stateID, Entity* _entity)
	: 
	State(stateID),
	m_entity(_entity)
{}

StateMedicDead::~StateMedicDead() {}

void StateMedicDead::Enter()
{
	m_entity->SetIsDead( true );
}

void StateMedicDead::Update(double dt)
{
	m_entity->SetHP(MEDIC_HP);
	m_entity->SetMaxHP(MEDIC_HP);
	m_entity->SetRecoveredHP(MEDIC_RECOVERED_HP);

	m_entity->SetVisibilityRange(MEDIC_RANGE);
	m_entity->SetFOV(MEDIC_FOV);

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

void StateMedicDead::UpdateTurn()
{
}

void StateMedicDead::Exit()
{
}
#pragma endregion
