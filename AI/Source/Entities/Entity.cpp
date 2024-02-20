#include "Entity.h"
#include "../ConcreteMessages.h"
#include "../SceneData.h"
#include "../Application.h"

#include "../States/StatesSoldier.h"
#include "../States/StatesArcher.h"
#include "../States/StatesMedic.h"
#include "../States/StatesTank.h"

#include <algorithm>
#include <random>
#include <queue>

#define deltaTime static_cast<float>(dt)

int Entity::m_nextValidID = 0;

static PostOffice& PO = *PostOffice::GetInstance();
static SceneData& data = *SceneData::GetInstance();

Entity::Entity(TYPE _type, Team _team) 
	:
	m_ID( ++m_nextValidID ),
	m_type( _type ),
	m_team( _team ),
	m_isActive( false ),
	m_isDead( true )
{
	m_HP			= 0.0f;
	m_MaxHP			= 0.0f;
	m_recoveredHP	= 0.0f;
	m_lifeTime		= 0.0f;
	m_other			= nullptr;
	m_turnsToMove   = 0;

	m_currPt		= MazePt(0, 0);
	m_visibleRange  = 1;	
	m_FOV			= 1;
	
	int size = data.GetNumGrid() * data.GetNumGrid();
	m_visited.resize(size, false);
	m_stack.clear();
	m_previous.resize( size );
	m_shortestPath.clear();
	m_visibleTiles.clear();

	// Each entity will have a unique order of
	// checking surrounding tiles during DFS
	int maxDir = static_cast<int>( Maze::DIRECTION::TOTAL );
	for (int i = 0; i < maxDir; ++i)
	{
		Maze::DIRECTION dir = static_cast<Maze::DIRECTION>(i);
		m_dirList.emplace_back( dir );
	}
	std::random_shuffle(m_dirList.begin(), m_dirList.end());
	m_dir = m_dirList.front();

	m_stateMachine = nullptr;
	AssignStateMachine();

	PO.Register(std::to_string(m_ID), this);
}

Entity::~Entity()
{
	if (m_stateMachine != nullptr)
	{
		delete m_stateMachine;
		m_stateMachine = nullptr;
	}
}

void Entity::Init()
{
	switch (this->m_type)
	{
		case TYPE::SOLDIER: this->GetFSM()->SetNextState("SoldierInit"); break;
		case TYPE::ARCHER: this->GetFSM()->SetNextState("ArcherInit"); break;
		case TYPE::MEDIC: this->GetFSM()->SetNextState("MedicInit"); break;
		case TYPE::TANK: this->GetFSM()->SetNextState("TankInit"); break;
	}
}

void Entity::DFSOnce()
{
	this->m_stack.push_back( this->m_currPt );
	int currIDX = Transform::Grid2Dto1D( this->m_currPt );
	this->m_visited[currIDX] = true;

	int numDir = static_cast<int>(Maze::DIRECTION::TOTAL);

	for (Maze::DIRECTION moveDir : m_dirList)
	{
		MazePt currPt = this->m_currPt;
		Maze::DIRECTION dir = moveDir;
		Tile* pNextTile = data.GetWorldMaze()->GetNextTile(currPt, dir);

		if (pNextTile == nullptr) continue;

		int nextX = pNextTile->GetMazePt().x;
		int nextY = pNextTile->GetMazePt().y;
		int nextIDX = nextY * data.GetNumGrid() + nextX;

		if (!this->m_visited[nextIDX])
		{
			if (data.GetWorldMaze()->GetTile(nextIDX)->GetCost() > 0)
			{
				bool moved = this->Move( moveDir );
				return;
			}
		}
	}

	this->m_stack.pop_back();
	if (!this->m_stack.empty())
	{
		Tile* pCurrTile = data.GetWorldMaze()->GetTile( m_currPt );
		Tile* pNextTile = data.GetWorldMaze()->GetTile( this->m_stack.back() );
		Maze::DIRECTION moveDir = data.GetWorldMaze()->GetMoveDir( pCurrTile, pNextTile );

		bool moved = this->Move( moveDir );
		if (moved) this->m_stack.pop_back();
		return;
	}
}

bool Entity::AStar(Tile* pTargetTile)
{
	data.GetWorldMaze()->ResetPathfindingCosts();

	int size = data.GetNumGrid() * data.GetNumGrid();
	m_previous.clear();
	m_previous.resize( size );
	m_shortestPath.clear();

	std::vector<Tile*> tmp_openList;
	std::vector<Tile*> tmp_closedList;

	Tile* pStartTile = data.GetWorldMaze()->GetTile( this->m_currPt );
	float heuristic = data.GetWorldMaze()->ManhattanDistance( pStartTile, pTargetTile );

	pStartTile->SetGlobalCost( pStartTile->GetCost() );
	pStartTile->SetHeuristic( heuristic );
	pStartTile->SetFinalCost( pStartTile->CalcFinalCost() );
	tmp_openList.push_back( pStartTile );

	Tile* pCurrTile = nullptr;

	while (!tmp_openList.empty())
	{
		std::sort(tmp_openList.begin(), tmp_openList.end(), [](Tile* lhs, Tile* rhs)
		{
			return lhs->GetFinalCost() < rhs->GetFinalCost();
		});

		pCurrTile = tmp_openList.front();
		tmp_openList.erase( tmp_openList.begin() );
		tmp_closedList.push_back( pCurrTile );

		if (pCurrTile == nullptr) return false;

		// Path has been found!
		if (pCurrTile == pTargetTile)
		{
			MazePt startPt = pStartTile->GetMazePt();
			MazePt tmpPt = pCurrTile->GetMazePt();

			while (tmpPt.DoesNotEqual( startPt ))
			{
				m_shortestPath.insert( m_shortestPath.begin(), tmpPt );
				tmpPt = this->m_previous[Transform::Grid2Dto1D(tmpPt)];
			}
			return true;
		}

		int numDir = static_cast<int>( Maze::DIRECTION::TOTAL );

		for (int i = 0; i < numDir; ++i)
		{
			Maze::DIRECTION dir = static_cast<Maze::DIRECTION>( i );

			Tile* pNextTile = data.GetWorldMaze()->GetNextTile( pCurrTile->GetMazePt(), dir );

			// Exit conditions
			if (pNextTile == nullptr) continue;
			if (pNextTile->GetCost() <= 0) continue;
			if (!pNextTile->GetIsVisited()) continue;

			auto itr = std::find(tmp_closedList.begin(), tmp_closedList.end(), pNextTile);
			bool inClosedList = (itr != tmp_closedList.end());
			if (inClosedList) continue;

			// Actual pathfinding stuffs
			float gCost = pCurrTile->GetGlobalCost() + pNextTile->GetCost();
			float hCost = data.GetWorldMaze()->ManhattanDistance( pNextTile, pTargetTile );
			float fCost = gCost + hCost;

			itr = std::find(tmp_openList.begin(), tmp_openList.end(), pNextTile);
			bool inOpenList = (itr != tmp_openList.end());
			
			if (fCost < pNextTile->GetFinalCost() || !inOpenList)
			{
				pNextTile->SetGlobalCost( gCost );
				pNextTile->SetHeuristic( hCost );
				pNextTile->SetFinalCost( fCost );

				int nextIDX = Transform::Grid2Dto1D( pNextTile->GetMazePt() );
				m_previous[nextIDX] = pCurrTile->GetMazePt();

				if (!inOpenList) tmp_openList.push_back( pNextTile );
			}
		}
	}
}

bool Entity::Handle(Message* msg)
{
	MessageAttack* msgATK = dynamic_cast<MessageAttack*>( msg );
	if (msgATK)
	{
		this->m_HP -= msgATK->damage;

		if (m_HP <= 0.0f)
		{
			Team teamOther = (m_team == 1) ? 2 : 1;
			MessageIncrementKillCount* msg = new MessageIncrementKillCount( teamOther );
			PO.Send("SceneA2", msg);
		}

		return true;
	}
	MessageHeal* msgHeal = dynamic_cast<MessageHeal*>( msg );
	if (msgHeal)
	{
		this->m_HP += msgHeal->heal;
		return true;
	}

	return false;
}

bool Entity::IsSameTeamAs(const Entity* other) const
{
	return m_team == other->GetTeam();
}

bool Entity::Move(const Maze::DIRECTION _dir)
{
	if (m_turnsToMove > 0)
	{
		--m_turnsToMove;
		return false;
	}

	Tile* pNextTile = data.GetWorldMaze()->GetNextTile(this->m_currPt, _dir);
	if (pNextTile == nullptr) return false;
	if (pNextTile->GetCost() <= 0) return false;

	m_dir = _dir;
	m_currPt.Set( pNextTile->GetMazePt() );
	m_visited[Transform::Grid2Dto1D(this->m_currPt)] = true;
	m_turnsToMove += data.GetWorldMaze()->GetTile(this->m_currPt)->GetCost() - 1;
	return true;

	// Shouldn't reach here.....
	return false;
}

bool Entity::Move(const MazePt& _targetPt)
{
	Maze::DIRECTION bestDir = data.GetWorldMaze()->GetMoveDir( this->m_currPt, _targetPt );
	bool moved = this->Move( bestDir );
	return moved;
}

std::vector<Tile*> Entity::CalcVisibleTiles()
{
	// Uses BFS to get all tiles within range
	// 
	// Then culls out certain tiles depending on the 
	// entities' FOV + whether they are behind another tile
	Tile* pStartTile = data.GetWorldMaze()->GetTile( this->m_currPt );
	Tile* currTile = data.GetWorldMaze()->GetTile( this->m_currPt );
	int size = data.GetNumGrid() * data.GetNumGrid();

	std::map<Tile*, int> tmp_blockedMap;	// Tile*, tile depth
	std::set<Tile*> tmp_tilesToCull;
	std::vector<bool> tmp_visited( size, false );
	std::queue<Tile*> tmp_queue;
	std::queue<int> depthQueue;
	std::vector<Maze::DIRECTION> tmp_dirToCheck;
	m_visibleTiles.clear();

	tmp_queue.push( currTile );
	m_visibleTiles.push_back( currTile );

	int startIDX = Transform::Grid2Dto1D( this->m_currPt );
	tmp_visited[startIDX] = true;

	int numDir = static_cast<int>( Maze::DIRECTION::TOTAL );
	int currDepth = 0;
	depthQueue.push( currDepth );

	for (int i = 0; i < numDir; ++i)
	{
		Maze::DIRECTION dir = static_cast<Maze::DIRECTION>(i);

		int dirDiff = data.GetWorldMaze()->GetDirDiff(this->m_dir, dir);
		if (dirDiff <= this->m_FOV - 1)
			tmp_dirToCheck.push_back( dir );
	}

	while (!tmp_queue.empty())
	{
		currTile = tmp_queue.front();
		tmp_queue.pop();

		currDepth = depthQueue.front();
		depthQueue.pop();

		if (currDepth == this->m_visibleRange) break;

		MazePt currTilePt = currTile->GetMazePt();
		int idx = Transform::Grid2Dto1D( currTilePt );

		for (Maze::DIRECTION checkDir : tmp_dirToCheck)
		{
			Tile* pNextTile = data.GetWorldMaze()->GetNextTile(currTilePt, checkDir); 

			if (pNextTile == nullptr)
				continue;

			if (pNextTile->GetContent() == Tile::CONTENT::WALL)
			{
				// pNextTile->SetIsVisited( true );
				tmp_blockedMap.insert(std::make_pair(pNextTile, currDepth + 1));
				continue;
			}

			for (auto itr : tmp_blockedMap)
			{
				// Use line-circle intersection to check if being blocked
				int blockingTileDepth = static_cast<int>(itr.second);
				if (blockingTileDepth > currDepth) continue;

				Tile* pBlockingTile = static_cast<Tile*>(itr.first);
				Vector3 hexPos = pBlockingTile->GetWorldPt();
				float radius = data.GetGridOffset() * 1.1f;

				Vector3 rayOrigin = pStartTile->GetWorldPt();
				Vector3 rayDir = pNextTile->GetWorldPt() - pStartTile->GetWorldPt();
				try {
					rayDir.Normalize();
				}
				catch (DivideByZero exception) {}
			
				Vector3 displacement = hexPos - rayOrigin;
				float proj = displacement.Dot( rayDir );

				// Trivial rejection test
				if (proj <= 0.001f)
					continue;

				Vector3 intersectionPt = rayOrigin + (rayDir * proj);
				float dist = (intersectionPt - hexPos).Length();

				if (dist <= radius)
					tmp_tilesToCull.insert( pNextTile );
			}

			MazePt nextPt = pNextTile->GetMazePt();
			int nextIDX = Transform::Grid2Dto1D( nextPt );

			if (!tmp_visited[nextIDX])
			{
				depthQueue.push(currDepth + 1);
				tmp_queue.push( pNextTile );
				tmp_visited[nextIDX] = true;
				m_visibleTiles.push_back( pNextTile );
			}
		}
	}

	for (auto pTile : tmp_tilesToCull)
	{
		auto itr = std::find(m_visibleTiles.begin(), m_visibleTiles.end(), pTile);
		bool found = (itr != m_visibleTiles.end());
		if (found) m_visibleTiles.erase( itr );
	}

	for (auto pTile : m_visibleTiles)
		pTile->SetIsVisited( true );

	for (auto itr : tmp_blockedMap)
	{
		auto find = std::find(m_visibleTiles.begin(), m_visibleTiles.end(), itr.first);
		bool found = (find != m_visibleTiles.end());
		if (found) m_visibleTiles.erase( find );
	}

	return m_visibleTiles;
}

// Getter/Setters (In retrospect there are too many, but it's too late at this point....)
int Entity::GetID() const			 { return m_ID; }
Entity::TYPE Entity::GetType() const { return m_type; }
Team Entity::GetTeam() const		 { return m_team; }

bool Entity::IsActive() const			 { return m_isActive; }
void Entity::SetIsActive(bool _isActive) { m_isActive = _isActive; }

bool Entity::IsDead() const			 { return m_isDead; }
void Entity::SetIsDead(bool _isDead) { m_isDead = _isDead; }

MazePt Entity::GetMazePt() const			  { return m_currPt; }
void Entity::SetMazePt(const MazePt& _mazePt) { m_currPt.Set( _mazePt ); }
void Entity::SetMazePt(int _x, int _y)		  { m_currPt.Set( _x, _y ); }

float Entity::GetHP() const    { return m_HP; }
void Entity::SetHP(int _HP)	   { m_HP = _HP; }
float Entity::GetMaxHP() const { return m_MaxHP; }
void Entity::SetMaxHP(int _HP) { m_MaxHP = _HP; }
float Entity::GetRecoveredHP() const		  { return m_recoveredHP; }
void Entity::SetRecoveredHP(int _recoveredHP) { m_recoveredHP = _recoveredHP; }

Entity* Entity::GetOtherEntity() const		 { return m_other; }
void Entity::SetOtherEntity(Entity* _entity) { m_other = _entity; }

Maze::DIRECTION Entity::GetDir() const			{ return m_dir; }
void Entity::SetDir(const Maze::DIRECTION _dir) { m_dir = _dir; }

int Entity::GetVisibilityRange() const		   { return m_visibleRange; }
void Entity::SetVisibilityRange(int _visRange) { m_visibleRange = _visRange; }

int Entity::GetFOV() const	  { return m_FOV; }
void Entity::SetFOV(int _fov) { m_FOV = _fov; }

StateMachine* Entity::GetFSM() const { return m_stateMachine; }

void Entity::AssignStateMachine()
{
	switch (this->m_type)
	{
		case TYPE::SOLDIER:
		{
			m_stateMachine = new StateMachine();
			m_stateMachine->AddState(new StateSoldierInit("SoldierInit", this));
			m_stateMachine->AddState(new StateSoldierPatrol("SoldierPatrol", this));
			m_stateMachine->AddState(new StateSoldierChase("SoldierChase", this));
			m_stateMachine->AddState(new StateSoldierAttack("SoldierAttack", this));
			m_stateMachine->AddState(new StateSoldierRetreat("SoldierRetreat", this));
			m_stateMachine->AddState(new StateSoldierDead("SoldierDead", this));
			m_stateMachine->SetNextState("SoldierInit");
			break;
		}
		case TYPE::ARCHER:
		{
			m_stateMachine = new StateMachine();
			m_stateMachine->AddState(new StateArcherInit("ArcherInit", this));
			m_stateMachine->AddState(new StateArcherPatrol("ArcherPatrol", this));
			m_stateMachine->AddState(new StateArcherChase("ArcherChase", this));
			m_stateMachine->AddState(new StateArcherAttack("ArcherAttack", this));
			m_stateMachine->AddState(new StateArcherRetreat("ArcherRetreat", this));
			m_stateMachine->AddState(new StateArcherDead("ArcherDead", this));
			m_stateMachine->SetNextState("ArcherInit");
			break;
		}
		case TYPE::MEDIC:
		{
			m_stateMachine = new StateMachine();
			m_stateMachine->AddState(new StateMedicInit("MedicInit", this));
			m_stateMachine->AddState(new StateMedicPatrol("MedicPatrol", this));
			m_stateMachine->AddState(new StateMedicChase("MedicChase", this));
			m_stateMachine->AddState(new StateMedicHeal("MedicHeal", this));
			m_stateMachine->AddState(new StateMedicRetreat("MedicRetreat", this));
			m_stateMachine->AddState(new StateMedicDead("MedicDead", this));
			m_stateMachine->SetNextState("MedicInit");
			break;
		}
		case TYPE::TANK:
		{
			m_stateMachine = new StateMachine();
			m_stateMachine->AddState(new StateTankInit("TankInit", this));
			m_stateMachine->AddState(new StateTankPatrol("TankPatrol", this));
			m_stateMachine->AddState(new StateTankChase("TankChase", this));
			m_stateMachine->AddState(new StateTankAttack("TankAttack", this));
			m_stateMachine->AddState(new StateTankRetreat("TankRetreat", this));
			m_stateMachine->AddState(new StateTankDead("TankDead", this));
			m_stateMachine->SetNextState("TankInit");
			break;
		}
	}
}

void Entity::PrintDir(const Maze::DIRECTION dir) const
{
	switch ( dir )
	{
		case Maze::DIRECTION::UP: std::cout << "Up " << std::endl; break;
		case Maze::DIRECTION::DOWN: std::cout << "Down " << std::endl; break;
		case Maze::DIRECTION::LEFTUP: std::cout << "Leftup " << std::endl; break;
		case Maze::DIRECTION::LEFTDOWN: std::cout << "Leftdown " << std::endl; break;
		case Maze::DIRECTION::RIGHTUP: std::cout << "Rightup " << std::endl; break;
		case Maze::DIRECTION::RIGHTDOWN: std::cout << "Rightdown " << std::endl; break;
		default: std::cout << "Entity: PrintDir() Error " << std::endl;
	}
}





