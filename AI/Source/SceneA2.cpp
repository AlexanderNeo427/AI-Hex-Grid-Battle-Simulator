#include "SceneA2.h"
#include "GL\glew.h"
#include "Application.h"
#include "Utility.h"
#include "Maze/MazeGenerator.h"

#define deltaTime static_cast<float>(dt)

static PostOffice& PO = *PostOffice::GetInstance();
static SceneData& data = *SceneData::GetInstance();

SceneA2::SceneA2()
{
}

SceneA2::~SceneA2()
{
}

void SceneA2::Init()
{
	SceneBase::Init();

	// Calculating aspect ratio
	float m_worldHeight = 100.f;
	float m_worldWidth = m_worldHeight * static_cast<float>(Application::GetWindowWidth()) /
										 static_cast<float>(Application::GetWindowHeight());	

	Math::InitRNG();

	// Init vars
	m_GameIsOver = false;
	m_scoreTeam1 = m_scoreTeam2 = 0;
	m_numTeam1   = m_numTeam2   = 0;
	m_winner	 = 0;

	m_timeTillNextState = 1.0f;
	m_timeTillNextEvent = 8.5f;

	m_speed		 = 1.f;
	m_bounceTime = 0.f;
	m_numTurn	 = MAX_TURNS;
	m_currTeam   = 1;

	m_updateState = UPDATE_STATE::ENTER;

	data.SetWorldSize(m_worldWidth, m_worldHeight);
	data.SetNumGrid( 20 );
	data.SetGridSize( data.GetWorldHeight() / data.GetNumGrid() );
	data.SetGridSize( data.GetGridSize() * 1.1185f );
	data.SetGridOffset( data.GetGridSize() / 2.0f );
	m_vecVisibleTiles.reserve( 200 );

	m_currMazeIdx = 0;
	m_mazeParams =
	{
		// Octaves, persistance (for Perlin)
		MazeParameters(11, 0.9537f),
		MazeParameters(5 , 0.7496f),
		MazeParameters(8 , 0.6300f),
	};
	m_maze = new Maze(data.GetNumGrid(), data.GetNumGrid(), Tile::CONTENT::EMPTY);
	MazeGenerator::PerlinGenerate(m_mazeParams[m_currMazeIdx].m_numOctaves,
								  m_mazeParams[m_currMazeIdx].m_persistance,
								  m_maze);
	data.SetWorldMaze( m_maze );

	// Pre-warming the entity pool
	m_EntityList.reserve( 450 );
	for (int team = 1; team <= 2; ++team)
	{
		int max = static_cast<int>(Entity::TYPE::TOTAL);

		for (int i = 0; i < max; ++i)
		{
			Entity::TYPE type = static_cast<Entity::TYPE>(i);

			for (int numEnts = 0; numEnts < 33; ++numEnts)
			{
				if (type == Entity::TYPE::TOTAL) 
					continue;

				m_EntityList.emplace_back(new Entity(type, team));
			}
		}
	}

	// Initialize some entities
	int entityCount = 0;
	for (Team team = 1; team <= 2; ++team)
	{
		int numTypes = static_cast<int>(Entity::TYPE::TOTAL);
	
		for (int i = 0; i < numTypes; ++i)
		{
			for (int numEnts = 0; numEnts < 2; ++numEnts)
			{
				Entity::TYPE type = static_cast<Entity::TYPE>( i );
	
				// Exclusion list
				if (type == Entity::TYPE::ARROW		|| type == Entity::TYPE::GRENADE ||
					type == Entity::TYPE::EXPLOSION || type == Entity::TYPE::TOTAL) 
					continue;
	
				FetchEntity(type, team);
				++entityCount;
			}
		}
	}
	std::cout << "Initial entity count: " << entityCount << std::endl;

	PO.Register("SceneA2", this);
}

void SceneA2::Update(double dt)
{
	SceneBase::Update( dt );

	//----------------------- Game over / Reset -------------------------
	if (m_GameIsOver)
	{
		dt = 0.0f;

		if (Application::IsKeyPressed('R'))
			ResetGame();

		return;
	}
	//------------------------ Input Section ---------------------------
	static bool bRButtonState = false;
	if (Application::IsKeyPressed('R') && !bRButtonState)
	{
		bRButtonState = true;
		ResetGame();
		return;
	}
	else if (!Application::IsKeyPressed('R') && bRButtonState)
	{
		bRButtonState = false;
	}

	// Increase/decrease simulation speed
	if (Application::IsKeyPressed(VK_OEM_PLUS))
	{
		m_speed = Math::Min(m_speed + 0.1f, 30.f);
	}
	if (Application::IsKeyPressed(VK_OEM_MINUS))
	{
		m_speed = Math::Max(m_speed - 0.1f, Math::EPSILON);
	}

	// Cycling between mazes
	static bool bChangeMaze			= false;
	static bool bLeftArrowKeyState  = false;
	static bool bRightArrowKeyState = false;

	if (Application::IsKeyPressed(VK_LEFT) && !bLeftArrowKeyState)
	{
		--m_currMazeIdx;
		if (m_currMazeIdx < 0)
			m_currMazeIdx = 2;

		bChangeMaze = true;
		bLeftArrowKeyState = true;
	}
	else if (bLeftArrowKeyState && !Application::IsKeyPressed(VK_LEFT))
	{
		bLeftArrowKeyState = false;
	}
	if (Application::IsKeyPressed(VK_RIGHT) && !bRightArrowKeyState)
	{
		++m_currMazeIdx;
		if (m_currMazeIdx > 2)
			m_currMazeIdx = 0;

		bChangeMaze = true;
		bRightArrowKeyState = true;
	}
	else if (bRightArrowKeyState && !Application::IsKeyPressed(VK_RIGHT))
	{
		bRightArrowKeyState = false;
	}

	if (bChangeMaze)
	{
		bChangeMaze = false;
		MazeGenerator::PerlinGenerate(m_mazeParams[m_currMazeIdx].m_numOctaves,
									  m_mazeParams[m_currMazeIdx].m_persistance,
									  m_maze);
		ResetGame();
		return;
	}

	//------------------------- Entity Update -------------------------------
	m_numTeam1 = 0;
	m_numTeam2 = 0;

	for (int i = 0; i < m_EntityList.size(); ++i)
	{
		Entity* entity = static_cast<Entity*>(m_EntityList[i]);

		if (!entity->IsActive()) continue;
		
		if (entity->GetFSM() != nullptr)
			entity->GetFSM()->Update(dt * m_speed);

		if (entity->GetType() == Entity::TYPE::EXPLOSION)
		{
			entity->m_lifeTime -= dt * m_speed;
			if (entity->m_lifeTime <= 0.0f)
				entity->SetIsActive( false );
		}

		// Keep count of Entities
		if (entity->GetType() == Entity::TYPE::SOLDIER || entity->GetType() == Entity::TYPE::ARCHER ||
			entity->GetType() == Entity::TYPE::MEDIC   || entity->GetType() == Entity::TYPE::TANK)
		{
			switch (entity->GetTeam())
			{
				case 1: ++m_numTeam1; break;
				case 2: ++m_numTeam2; break;
			}
		}
	}
	//----------------------- Update turn --------------------------
	if (m_timeTillNextState <= 0.0f)
	{
		switch (m_updateState)
		{
		case UPDATE_STATE::ENTER:
			m_timeTillNextState = 0.35f;
			m_updateState = UPDATE_STATE::MOVE;
			m_setVisibleTiles = GetVisibleTiles( m_currTeam );
			break;
		case UPDATE_STATE::MOVE:
			UpdateTurn( m_currTeam );
			m_setVisibleTiles = GetVisibleTiles(m_currTeam);
			--m_numTurn;
			m_timeTillNextState = 0.35f;
			m_updateState = UPDATE_STATE::PAUSE;
			break;
		case UPDATE_STATE::PAUSE:
			m_timeTillNextState = 0.35f;
			m_updateState = UPDATE_STATE::SWITCH_TURN;
			break;
		case UPDATE_STATE::SWITCH_TURN:
			switch (m_currTeam)
			{
				case 1: m_currTeam = 2; break;
				case 2: m_currTeam = 1; break;
			}
			m_updateState = UPDATE_STATE::ENTER;
			m_timeTillNextState = 0.35f;
			break;
		}
	}
	m_timeTillNextState -= deltaTime * m_speed;

	//----------------------- Random Events --------------------------
	m_timeTillNextEvent -= deltaTime * m_speed;
	if (m_timeTillNextEvent <= 0.0f)
	{
		m_timeTillNextEvent = Math::RandFloatMinMax(2.5f, 6.f);

		float chance = Math::RandFloatMinMax(0.f, 100.f);
		if (chance <= EVENT_CHANCE)
		{
			int numEvents = static_cast<int>(EVENT::NUM_TOTAL) - 1;
			int randNum = Math::RandIntMinMax(0, numEvents);
			EVENT randomEvent = static_cast<EVENT>(randNum);

			switch ( randomEvent )
			{
				case EVENT::METEOR:
				{
					int randX = Math::RandIntMinMax(0, data.GetNumGrid() - 1);
					int randY = Math::RandIntMinMax(0, data.GetNumGrid() - 1);
					int radius = Math::RandIntMinMax(1, 3);
					float randDMG = Math::RandFloatMinMax(25.f, 85.f);

					std::vector<Tile*> listAffectedTiles = TilesWithinRange(MazePt(randX, randY), radius);
					for (Tile* pTile : listAffectedTiles)
					{
						MazePt pt = pTile->GetMazePt();
						Entity* boom = FetchEntity(Entity::TYPE::EXPLOSION, 3);
						boom->SetMazePt( pt );
						boom->m_lifeTime = 1.7f;

						// Dmg entities within range
						for (Entity* entity : m_EntityList)
						{
							if (!entity->IsActive()) continue;
							if (entity->IsDead()) continue;
							if (entity->IsSameTeamAs( boom )) continue;
							if (entity->GetType() == boom->GetType()) continue;

							if (entity->GetMazePt().IsEqual( pt ))
							{
								MessageAttack* msgATK = new MessageAttack( randDMG );
								std::string entityAddr = std::to_string(entity->GetID());
								PO.Send(entityAddr, msgATK);
							}
						}
					}
					std::cout << "Meteor strike event " << std::endl;
					std::cout << "Position: " << randX << ", " << randY << std::endl;
					std::cout << "Blast radius: " << radius << std::endl << std::endl;
					break;
				}
				case EVENT::DEATH:
				{	
					std::vector<Entity*> tmp_EntityList;
					for (Entity* pEntity : m_EntityList)
					{
						if (!pEntity->IsActive()) continue;
						if (pEntity->IsDead()) continue;
						if (pEntity->GetTeam() != 1 && pEntity->GetTeam() != 2)
							continue;

						if (pEntity->GetType() == Entity::TYPE::SOLDIER || pEntity->GetType() == Entity::TYPE::ARCHER ||
							pEntity->GetType() == Entity::TYPE::MEDIC	|| pEntity->GetType() == Entity::TYPE::TANK)
						{
							tmp_EntityList.push_back( pEntity );
						}
					}
					int randIDX = Math::RandIntMinMax(0, tmp_EntityList.size() - 1);
					tmp_EntityList[randIDX]->SetHP( -0.001f );
					Entity* redTile = FetchEntity(Entity::TYPE::EXPLOSION, 3);
					redTile->SetMazePt(tmp_EntityList[randIDX]->GetMazePt());
					redTile->m_lifeTime = 1.7f;

					std::cout << "Random death event " << std::endl;
					std::cout << "Entity ID: " << tmp_EntityList[randIDX]->GetID() << std::endl;
					std::string team = (tmp_EntityList[randIDX]->GetTeam() == 1 ? "Red " : "Blue ");
					std::cout << "Entity Team: " << team << std::endl << std::endl;
					break;
				}
				case EVENT::SPAWN:
				{
					std::vector<Entity::TYPE> typeWhiteList = 
					{
						Entity::TYPE::SOLDIER,
						Entity::TYPE::ARCHER,
						Entity::TYPE::MEDIC,
						Entity::TYPE::TANK,
					};
					int randNumSpawn = Math::RandIntMinMax(1, 3);
					for (int i = 0; i < randNumSpawn; ++i)
					{
						float chance = Math::RandFloatMinMax(0.f, 100.f);
						Team spawnTeam = (m_numTeam1 < m_numTeam2 ? 1 : 2);
						int numTypes = Math::RandIntMinMax(0, typeWhiteList.size() - 1);
						Entity::TYPE randType = static_cast<Entity::TYPE>(typeWhiteList[numTypes]);

						FetchEntity(randType, spawnTeam);
					}

					std::cout << "Random spawn(s) event " << std::endl;
					std::cout << "Num. of new Entities: " << randNumSpawn << std::endl << std::endl;
					break;
				}
			}
		}
	}

	//----------------------- Game end conditions -------------------------
	if (m_numTurn == 0)
	{
		m_GameIsOver = true;

		if (m_scoreTeam1 == m_scoreTeam2)
			m_winner = 0;
		else if (m_scoreTeam1 > m_scoreTeam2)
			m_winner = 1;
		else
			m_winner = 2;
	}
	if (m_scoreTeam1 >= WIN_SCORE)
	{
		m_GameIsOver = true;
		m_winner = 1;
	}
	if (m_scoreTeam2 >= WIN_SCORE)
	{
		m_GameIsOver = true;
		m_winner = 2;
	}
	if (m_numTeam1 == 0)
	{
		m_GameIsOver = true;
		m_winner = 2;
	}
	if (m_numTeam2 == 0)
	{
		m_GameIsOver = true;
		m_winner = 2;
	}
}

void SceneA2::UpdateTurn(const Team teamNumber)
{
	for (Entity* ent : m_EntityList)
	{
		if (ent->IsActive() && ent->GetTeam() == teamNumber)
		{
			if (ent->GetFSM() != nullptr)
			{
				ent->GetFSM()->UpdateTurn();

				MazePt mazePt = ent->GetMazePt();
				data.GetWorldMaze()->GetTile( mazePt )->SetIsVisited( true );
			}
		}
	}
}

void SceneA2::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, data.GetWorldWidth(), 0, data.GetWorldHeight(), -10, 10);
	projectionStack.LoadMatrix(projection);
	
	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
						camera.position.x, camera.position.y, camera.position.z,
						camera.target.x, camera.target.y, camera.target.z,
						camera.up.x, camera.up.y, camera.up.z
					);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();
	
	RenderMesh(meshList[GEO_AXES], false);

	float bgWidth = data.GetWorldHeight() * 0.9f;
	float bg_posX = bgWidth * 0.5f;

	modelStack.PushMatrix();
		modelStack.Translate(bg_posX, data.GetWorldHeight() * 0.5f, -1.f);
		modelStack.Scale(bgWidth, data.GetWorldHeight(), 0.0001f);
		RenderMesh(meshList[GEO_BLACKQUAD], false);
	modelStack.PopMatrix();

	// On-screen text
	std::ostringstream ss;

	// Render tiles
	for (Tile* tile : data.GetWorldMaze()->GetGrid())
		RenderTile( tile, false );

	// Render visibility tiles
	for (Tile* tile : m_setVisibleTiles)
		RenderTile( tile, true );

	// Render entities
	float zOffset = 0.004f;
	for (Entity* entity : m_EntityList)
	{
		if (!entity->IsActive()) continue;

		RenderEntity( entity, zOffset );

		if (entity->GetFSM() != nullptr)
		{
			RenderEntityHP(entity, zOffset);

			if (entity->GetTeam() == m_currTeam)
				RenderEntityState( entity );
		}
		
		// Render AStar shortest paths of all entities
		float indicatorSize = data.GetGridOffset() * 0.6f;
		if (!entity->IsDead() && !entity->m_shortestPath.empty() && entity->GetTeam() == m_currTeam)
		{
			if (entity->GetFSM() == nullptr) continue;
				
			for (MazePt& pt : entity->m_shortestPath)
			{
				Vector3 pos = Transform::MazeToWorldPt( pt );

				modelStack.PushMatrix();
					modelStack.Translate(pos.x, pos.y, 0.1f);
					modelStack.Scale(indicatorSize, indicatorSize, 0.0f);
					switch (entity->GetTeam())
					{
						case 1: RenderMesh(meshList[GEO_RED_ORB], false); break;
						case 2: RenderMesh(meshList[GEO_BLUE_ORB], false); break;
					}
				modelStack.PopMatrix();
			}
		}

		zOffset += Math::EPSILON;
	}

	if (m_GameIsOver)
	{
		std::string gameOverText = "";
		switch (m_winner)
		{
			case 0: gameOverText = "Tie!";		  break;
			case 1: gameOverText = "Team 1 wins"; break;
			case 2: gameOverText = "Team 2 wins"; break;
		}
		ss.str("");
		ss << gameOverText;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 10.f, 25, 24.5f);
	}

	ss.str("");
	ss << "Current team: " << (m_currTeam == 1 ? "Red" : "Blue");
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.2f, 54, 40.0f);

	ss.str("");
	ss << "Red score:  " << m_scoreTeam1;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.2f, 54, 35.f);

	ss.str("");
	ss << "Blue score:  " << m_scoreTeam2;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.2f, 54, 33.f);

	ss.str("");
	ss.precision(3);
	ss << "Remaining turns: " << m_numTurn;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.2f, 54, 28.f);

	ss.str("");
	ss.precision(3);
	ss << "Maze: " << m_currMazeIdx;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.2f, 54, 17.f);

	ss.str("");
	ss.precision(3);
	ss << "Speed: " << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.2f, 54, 8.5f);

	ss.str("");
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5f, 54, 5.5f);
	
	RenderTextOnScreen(meshList[GEO_TEXT], "AI Assignment 2", Color(0, 1, 0), 3.0f, 54, 0.5f);
}

void SceneA2::Exit()
{
	SceneBase::Exit();

	while (m_EntityList.size() > 0)
	{
		delete m_EntityList.back();
		m_EntityList.back() = nullptr;
		m_EntityList.pop_back();
	}

	if (data.GetWorldMaze() != nullptr)
	{
		delete data.GetWorldMaze();
		data.SetWorldMaze( nullptr );
	}
}

void SceneA2::RenderTile(Tile* tile, bool tileIsVisible)
{
	std::ostringstream ss;
	float tileRenderSize = data.GetGridSize() * 0.92f;

	if (tileIsVisible)
	{
		tile->SetIsVisited( true );
	}
	else
	{
		if (!tile->GetIsVisited())
		{
			modelStack.PushMatrix();
				modelStack.Translate(tile->GetWorldPt().x, tile->GetWorldPt().y, 0.001f);
				modelStack.Rotate(90.0f, 1.0f, 0.0f, 0.0f);
				modelStack.Scale(tileRenderSize, 0.0f, tileRenderSize);
				RenderMesh(meshList[GEO_BLACK_HEX], false);
			modelStack.PopMatrix();
			return;
		}
	}

	Mesh* tmpMesh = meshList[GEO_BLACK_HEX];

	if ( !tileIsVisible )
	{
		switch (tile->GetContent())
		{
			case Tile::CONTENT::WALL:  tmpMesh = meshList[GEO_GREY_HEX_MUTED];  break;
			case Tile::CONTENT::EMPTY: tmpMesh = meshList[GEO_WHITE_HEX_MUTED]; break;
			case Tile::CONTENT::GRASS: tmpMesh = meshList[GEO_GREEN_HEX_MUTED]; break;
			case Tile::CONTENT::WATER: tmpMesh = meshList[GEO_BLUE_HEX_MUTED];  break;
			case Tile::CONTENT::MUD:   tmpMesh = meshList[GEO_BROWN_HEX_MUTED]; break;
			// default: /*mesh = meshList[GEO_GREY_HEX_MUTED]; */ break;
		}
	}
	else
	{
		switch (tile->GetContent())
		{
			case Tile::CONTENT::WALL:  tmpMesh = meshList[GEO_GREY_HEX];  break;
			case Tile::CONTENT::EMPTY: tmpMesh = meshList[GEO_WHITE_HEX]; break;
			case Tile::CONTENT::GRASS: tmpMesh = meshList[GEO_GREEN_HEX]; break;
			case Tile::CONTENT::WATER: tmpMesh = meshList[GEO_BLUE_HEX];  break;
			case Tile::CONTENT::MUD:   tmpMesh = meshList[GEO_BROWN_HEX]; break;
			// default: /*mesh = meshList[GEO_GREY_HEX_MUTED]; */ break;
		}
	}

	float zOffset = tileIsVisible ? 0.0003f : 0.0002f;

	// Render tiles
	modelStack.PushMatrix(); 
		modelStack.Translate(tile->GetWorldPt().x, tile->GetWorldPt().y, zOffset);
		modelStack.Rotate(90.0f, 1.0f, 0.0f, 0.0f);
		modelStack.Scale(tileRenderSize, 0.0f, tileRenderSize);
		RenderMesh(tmpMesh, false);
	modelStack.PopMatrix();

	// Render tile coordinates
	//modelStack.PushMatrix();
	//	modelStack.Translate(tile->GetWorldPt().x - 0.7f, tile->GetWorldPt().y - 0.7f, 0.1f);
	//	modelStack.Scale(1.5f, 1.5f, 1.5f);
	//	ss.str("");
	//	ss << tile->GetMazePt().x << ", " << tile->GetMazePt().y;
	//	RenderText(meshList[GEO_TEXT], ss.str(), Color(0.f, 0.f, 0.f));
	//modelStack.PopMatrix();

	// Render tile indices
	/*modelStack.PushMatrix();
		modelStack.Translate(tile->GetWorldPt().x - 0.f, tile->GetWorldPt().y + 0.3f, 0.1f);
		modelStack.Scale(1.7f, 1.7f, 1.7f);
		ss.str("");
		int idx = (tile->GetMazePt().y * data.GetNumGrid()) + tile->GetMazePt().x;
		ss << idx;
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0.1f, 0.1f, 0.1f));
	modelStack.PopMatrix(); */
	
	// Render tile costs
	modelStack.PushMatrix();
		modelStack.Translate(tile->GetWorldPt().x, tile->GetWorldPt().y, 0.1f);
		modelStack.Scale(1.75f, 1.75f, 1.75f);
		ss.str("");
		ss << tile->GetCost();
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0));
	modelStack.PopMatrix();
}

Entity* SceneA2::FetchEntity(Entity::TYPE _type, Team _team)
{
	for (int i = 0; i < m_EntityList.size(); ++i)
	{
		Entity* entity = static_cast<Entity*>( m_EntityList[i] );

		if (!entity->IsActive() && entity->IsDead())
			if (entity->GetType() == _type && entity->GetTeam() == _team)
			{
				entity->Init();
				entity->SetIsActive( true );

				return entity;
			}
	}

	for (int i = 0; i < 10; ++i)
	{
		Entity* pNewEntity = new Entity(_type, _team);
		pNewEntity->Init();

		m_EntityList.push_back( pNewEntity );
	}

	return FetchEntity(_type, _team);
}

void SceneA2::RenderEntity(Entity* entity, float zOffset)
{
	Mesh* mesh = nullptr;
	Vector3 pos = Transform::MazeToWorldPt(entity->GetMazePt());
	float size = data.GetGridSize() * 0.6f;

	if (entity->GetType() == Entity::TYPE::EXPLOSION)
	{
		float tileRenderSize = data.GetGridSize() * 0.93f;

		modelStack.PushMatrix();
			modelStack.Translate(pos.x, pos.y, 0.002f);
			modelStack.Rotate(90.0f, 1.0f, 0.0f, 0.0f);
			modelStack.Scale(tileRenderSize, 0.0f, tileRenderSize);
			RenderMesh(meshList[GEO_RED_HEX], false);
		modelStack.PopMatrix();
		return;
	}

	if (entity->GetTeam() == 1)
	{
		switch (entity->GetType())
		{
			case Entity::TYPE::SOLDIER: mesh = meshList[GEO_SOLDIER_ATTACKER]; break;
			case Entity::TYPE::MEDIC:   mesh = meshList[GEO_MEDIC_ATTACKER]; break;
			case Entity::TYPE::ARCHER:  mesh = meshList[GEO_ARCHER_ATTACKER]; break;
			case Entity::TYPE::TANK:	mesh = meshList[GEO_TANK_ATTACKER]; break;
			default: std::cout << "RenderEntity() Error " << std::endl; break;
		}
	}
	else if (entity->GetTeam() == 2)
	{
		switch (entity->GetType())
		{
			case Entity::TYPE::SOLDIER: mesh = meshList[GEO_SOLDIER_DEFENDER]; break;
			case Entity::TYPE::MEDIC:   mesh = meshList[GEO_MEDIC_DEFENDER]; break;
			case Entity::TYPE::ARCHER:  mesh = meshList[GEO_ARCHER_DEFENDER]; break;
			case Entity::TYPE::TANK:    mesh = meshList[GEO_TANK_DEFENDER]; break;
			default: std::cout << "RenderEntity() Error " << std::endl; break;
		}
	}

	modelStack.PushMatrix();
		modelStack.Translate(pos.x, pos.y, zOffset);
		modelStack.Scale(size, size, 0.1f);
		RenderMesh(mesh, false);
	modelStack.PopMatrix();

	// Render turns to move
	/*std::ostringstream ss;
	modelStack.PushMatrix();
		modelStack.Translate(pos.x, pos.y - 1.3f, 0.1f);
		modelStack.Scale(3.f, 3.f, 3.f);
		ss.str("");
		ss << entity->m_turnsToMove;
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0.f, 1.f, 0.f));
	modelStack.PopMatrix();*/
}

void SceneA2::RenderEntityHP(Entity* entity, float zOffset)
{
	std::ostringstream ss;

	static const float HP_BAR_LENGTH = 4.0f;
	static const float HP_BAR_WIDTH = 1.0f;

	float displayHP = Math::Max(Math::EPSILON, entity->GetHP());
	float diff = entity->GetMaxHP() - displayHP;
	float ratio = diff / entity->GetMaxHP();
	float xOffset = (HP_BAR_LENGTH * ratio) / 2.f;

	Vector3 worldPos = Transform::MazeToWorldPt( entity->GetMazePt() );

	Mesh* meshHP = nullptr;
	switch (entity->GetTeam())
	{
		case 1: meshHP = meshList[GEO_ATTACK_HP]; break;
		case 2: meshHP = meshList[GEO_DEFEND_HP]; break;
	}

	modelStack.PushMatrix();
		modelStack.Translate(worldPos.x - xOffset, worldPos.y + 2.2f, zOffset);
		modelStack.Scale(HP_BAR_LENGTH * (displayHP / entity->GetMaxHP()), HP_BAR_WIDTH, 0.001f);
		RenderMesh(meshHP, false);
	modelStack.PopMatrix();
}

void SceneA2::RenderEntityState(Entity* entity)
{
	std::ostringstream ss;
	Vector3 pos = Transform::MazeToWorldPt(entity->GetMazePt());

	modelStack.PushMatrix();
		modelStack.Translate(pos.x - 2.5f, pos.y - 1.7f, 0.1f);
		modelStack.Scale(2.5f, 2.5f, 2.5f);
		ss.str("");
		ss << entity->GetFSM()->GetCurrentState();
		RenderText(meshList[GEO_TEXT], ss.str(), Color(0.f, 1.f, 0.f));
	modelStack.PopMatrix();
}

bool SceneA2::Handle(Message* msg)
{
	MessageEnemyInLOS* msgEnemyInRange = dynamic_cast<MessageEnemyInLOS*>(msg);
	if ( msgEnemyInRange )
	{
		Entity* pEntity = msgEnemyInRange->m_entity;
		Entity* pOther = nullptr;
		std::vector<Tile*> listVisibleTiles = pEntity->CalcVisibleTiles();

		float minDist = FLT_MAX;

		for (Entity* entItr : m_EntityList)
		{
			if (!entItr->IsActive()) continue;
			if (entItr->IsDead()) continue;
			if (entItr == pEntity) continue;
			if (entItr->IsSameTeamAs( pEntity )) continue;
			if (entItr->GetTeam() == 3) continue;

			MazePt enemyPt = entItr->GetMazePt();

			for (Tile* pTile : listVisibleTiles) 
			{
				if (pTile->GetMazePt().DoesNotEqual( enemyPt )) continue;

				float dist = data.GetWorldMaze()->ManhattanDistance( pEntity->GetMazePt(), enemyPt );
				if (dist < minDist)
				{
					minDist = dist;
					pOther = entItr;
				}
			}
		}

		if (pOther == nullptr) 
			return false;
		
		pEntity->SetOtherEntity( pOther );
		return true;
	}
	MessageMedicHelp* msgMedicHelp = dynamic_cast<MessageMedicHelp*>(msg);
	if (msgMedicHelp)
	{
		Entity* pRequester = msgMedicHelp->pRequester;

		for (Entity* pMedic : m_EntityList)
		{
			// Exit conditions
			if (!pMedic->IsActive()) continue;
			if (pMedic->IsDead()) continue;
			if (pMedic == pRequester) continue;
			if (!pMedic->IsSameTeamAs(pRequester)) continue;
			if (pMedic->GetType() != Entity::TYPE::MEDIC) continue;

			if (pMedic->GetFSM() != nullptr)
			{
				if (pMedic->GetFSM()->GetCurrentState() == "MedicPatrol")
				{
					pMedic->SetOtherEntity( pRequester );
					return true;
				}
			}
		}
		return false;
	}
	MessageCavalry* msgCavalry = dynamic_cast<MessageCavalry*>(msg);
	if (msgCavalry)
	{
		Entity* pRequester = msgCavalry->pRequester;

		for (Entity* pEntity : m_EntityList)
		{
			// Exit conditions
			if (!pEntity->IsActive()) continue;
			if (pEntity->IsDead()) continue;
			if (pEntity == pRequester) continue;
			if (!pEntity->IsSameTeamAs(pRequester)) continue;
			if (pEntity->GetType() == Entity::TYPE::MEDIC) continue;

			if (pEntity->GetFSM() != nullptr)
			{
				if (pEntity->GetFSM()->GetCurrentState() == "SoldierPatrol" ||
					pEntity->GetFSM()->GetCurrentState() == "ArcherPatrol" || 
					pEntity->GetFSM()->GetCurrentState() == "TankPatrol")
				{
					pEntity->SetOtherEntity( pRequester->GetOtherEntity() );
					return true;
				}
			}
		}
		return false;
	}
	MessageIncrementKillCount* msgKillCount = dynamic_cast<MessageIncrementKillCount*>(msg);
	if (msgKillCount)
	{
		switch (msgKillCount->team)
		{
			case 1: ++m_scoreTeam1; return true;
			case 2: ++m_scoreTeam2; return true;
		}
		return false;
	}
	return false;
}

void SceneA2::ResetGame()
{
	m_EntityList.clear();

	m_GameIsOver  = false;
	m_scoreTeam1  = 0;
	m_scoreTeam2  = 0;
	m_winner	  = 0;
	m_numTurn	  = MAX_TURNS;
	m_updateState = UPDATE_STATE::ENTER;

	m_timeTillNextState = 1.0f;
	m_timeTillNextEvent = 2.0f;

	m_speed		 = 1.0f;
	m_bounceTime = 0.0f;
	m_currTeam	 = 1;

	m_vecVisibleTiles.clear();
	m_setVisibleTiles.clear();

	data.GetWorldMaze()->ResetPathfindingCosts();

	for (Tile* pTile : data.GetWorldMaze()->GetGrid())
		pTile->SetIsVisited( false );

	for (int team = 1; team <= 2; ++team)
	{
		int numTypes = static_cast<int>(Entity::TYPE::TOTAL);

		for (int i = 0; i < numTypes; ++i)
		{
			for (int numEnts = 0; numEnts < 2; ++numEnts)
			{
				Entity::TYPE type = static_cast<Entity::TYPE>(i);

				// Exclusion list
				if (type == Entity::TYPE::ARROW		|| type == Entity::TYPE::GRENADE ||
					type == Entity::TYPE::EXPLOSION || type == Entity::TYPE::TOTAL)
					continue;

				FetchEntity(type, team);
			}
		}
	}
}

std::set<Tile*> SceneA2::GetVisibleTiles(const Team _team)
{
	m_vecVisibleTiles.clear();
	std::vector<Tile*> tmp_visibleTiles( 80 );

	for (Entity* entity : m_EntityList)
	{
		if (entity->IsActive() && entity->GetTeam() == _team && !entity->IsDead())
		{
			if (entity->GetType() == Entity::TYPE::SOLDIER || entity->GetType() == Entity::TYPE::ARCHER ||
				entity->GetType() == Entity::TYPE::MEDIC   || entity->GetType() == Entity::TYPE::TANK)
			{
				tmp_visibleTiles = entity->CalcVisibleTiles();
				m_vecVisibleTiles.insert(m_vecVisibleTiles.end(), tmp_visibleTiles.begin(), tmp_visibleTiles.end());
			}
		}
	}
	
	std::set<Tile*> visibleSet(m_vecVisibleTiles.begin(), m_vecVisibleTiles.end());
	return visibleSet;
}

std::vector<Tile*> SceneA2::TilesWithinRange(const MazePt& mazePt, int radius)
{
	// BFS with a queue to keep track of the depth
	int size = data.GetNumGrid() * data.GetNumGrid();
	std::vector<bool>  tmp_visited( size, false );
	std::vector<Tile*> tmp_affectedTiles;
	std::queue<Tile*>  tmp_queue;
	std::queue<int>    tmp_depthQueue;
	int currDepth = 0;

	Tile* pCurrTile = data.GetWorldMaze()->GetTile( mazePt );
	tmp_queue.push( pCurrTile );
	tmp_affectedTiles.push_back( pCurrTile );
	tmp_visited[Transform::Grid2Dto1D(mazePt)] = true;
	tmp_depthQueue.push( currDepth );

	int maxDir = static_cast<int>(Maze::DIRECTION::TOTAL);
	while (!tmp_queue.empty())
	{
		pCurrTile = tmp_queue.front();
		tmp_queue.pop();

		currDepth = tmp_depthQueue.front();
		tmp_depthQueue.pop();

		if (currDepth == radius)
			break;

		for (int i = 0; i < maxDir; ++i)
		{
			Maze::DIRECTION dir = static_cast<Maze::DIRECTION>(i);
			Tile* pNextTile = data.GetWorldMaze()->GetNextTile( pCurrTile->GetMazePt(), dir );
			if (pNextTile == nullptr)
				continue;

			MazePt nextPt = pNextTile->GetMazePt();
			int nextIDX = Transform::Grid2Dto1D( nextPt );

			if (!tmp_visited[nextIDX])
			{
				tmp_depthQueue.push(currDepth + 1);
				tmp_queue.push( pNextTile );
				tmp_visited[nextIDX] = true;
				tmp_affectedTiles.push_back( pNextTile );
			}
		}
	}

	return tmp_affectedTiles;
}
