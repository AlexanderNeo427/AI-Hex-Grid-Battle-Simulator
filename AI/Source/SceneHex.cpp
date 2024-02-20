//#include "SceneHex.h"
//#include "GL\glew.h"
//#include "Application.h"
//#include <sstream>
//
//SceneHex::SceneHex()
//{
//}
//
//SceneHex::~SceneHex()
//{
//}
//
//void SceneHex::Init()
//{
//	SceneBase::Init();
//
//	//Calculating aspect ratio
//	m_worldHeight = 100.f;
//	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
//
//	//Physics code here
//	m_speed = 1.f;
//
//	Math::InitRNG();
//
//	m_noGrid = 12;
//	m_gridSize = m_worldHeight / m_noGrid;
//	m_gridOffset = m_gridSize / 2;
//
//	m_start.Set(0, 0);
//	m_mazeKey = 0;
//	m_wallLoad = 0.3f;
//	m_maze.Generate(m_mazeKey, m_noGrid, m_start, m_wallLoad); //Generate new maze
//	m_myGrid.resize(m_noGrid * m_noGrid);
//	m_visited.resize(m_noGrid * m_noGrid);
//	m_previous.resize(m_noGrid * m_noGrid);
//	std::fill(m_myGrid.begin(), m_myGrid.end(), Maze::TILE_FOG);
//	std::fill(m_visited.begin(), m_visited.end(), false);
//	m_myGrid[m_start.y * m_noGrid + m_start.x] = Maze::TILE_EMPTY;
//	
//	Sleep( 3000 );
//	DFS(m_start);
//}
//
//GameObject* SceneHex::FetchGO()
//{
//	for (std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
//	{
//		GameObject *go = (GameObject *)*it;
//		if (!go->active)
//		{
//			go->active = true;
//			++m_objectCount;
//			return go;
//		}
//	}
//	for (unsigned i = 0; i < 10; ++i)
//	{
//		GameObject *go = new GameObject(GameObject::TYPE::NONE);
//		m_goList.push_back( go );
//	}
//	return FetchGO();
//}
//
//void SceneHex::DFS(MazePt curr)
//{
//	m_visited[curr.y * m_noGrid + curr.x] = true;
//	//UP
//	if(curr.y < m_noGrid - 1)
//	{
//		MazePt next(curr.x, curr.y + 1);
//		if (!m_visited[next.y * m_noGrid + next.x])
//		{
//			if (m_maze.Move(Maze::DIR_UP) == true)
//			{
//				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
//				DFS(next);
//				m_maze.Move(Maze::DIR_DOWN);
//			}
//			else
//			{
//				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
//			}
//		}
//	}
//	//DOWN
//	if (curr.y > 0)
//	{
//		MazePt next(curr.x, curr.y - 1);
//		if (!m_visited[next.y * m_noGrid + next.x])
//		{
//			if (m_maze.Move(Maze::DIR_DOWN) == true)
//			{
//				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
//				DFS(next);
//				m_maze.Move(Maze::DIR_UP);
//			}
//			else
//			{
//				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
//			}
//		}
//	}
//	//LEFT
//	if (curr.x > 0)
//	{
//		MazePt next(curr.x - 1, curr.y);
//		if (!m_visited[next.y * m_noGrid + next.x])
//		{
//			if (m_maze.Move(Maze::DIR_LEFT) == true)
//			{
//				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
//				DFS(next);
//				m_maze.Move(Maze::DIR_RIGHT);
//			}
//			else
//			{
//				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
//			}
//		}
//	}
//	//RIGHT
//	if (curr.x < m_noGrid - 1)
//	{
//		MazePt next(curr.x + 1, curr.y);
//		if (!m_visited[next.y * m_noGrid + next.x])
//		{
//			if (m_maze.Move(Maze::DIR_RIGHT) == true)
//			{
//				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
//				DFS(next);
//				m_maze.Move(Maze::DIR_LEFT);
//			}
//			else
//			{
//				m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
//			}
//		}
//	}
//	if (curr.x % 2 == 0) //even
//	{
//		if (curr.x > 0 && curr.y > 0) //leftdown
//		{
//			MazePt next(curr.x - 1, curr.y - 1);
//			if (!m_visited[next.y * m_noGrid + next.x])
//			{
//				if (m_maze.Move(Maze::DIR_LEFTDOWN) == true)
//				{
//					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
//					DFS(next);
//					m_maze.Move(Maze::DIR_RIGHTUP);
//				}
//				else
//				{
//					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
//				}
//			}
//		}
//
//		if (curr.x < m_noGrid - 1 && curr.y > 0) //rightdown
//		{
//			MazePt next(curr.x + 1, curr.y - 1);
//			if (!m_visited[next.y * m_noGrid + next.x])
//			{
//				if (m_maze.Move(Maze::DIR_RIGHTDOWN) == true)
//				{
//					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
//					DFS(next);
//					m_maze.Move(Maze::DIR_LEFTUP);
//				}
//				else
//				{
//					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
//				}
//			}
//		}
//	}
//	else
//	{
//		if (curr.x > 0 && curr.y < m_noGrid - 1)
//		{
//			MazePt next(curr.x - 1, curr.y + 1);
//			if (!m_visited[next.y * m_noGrid + next.x])
//			{
//				if (m_maze.Move(Maze::DIR_LEFTUP) == true)
//				{
//					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
//					DFS(next);
//					m_maze.Move(Maze::DIR_RIGHTDOWN);
//				}
//				else
//				{
//					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
//				}
//			}
//		}
//		if (curr.x < m_noGrid - 1 && curr.y < m_noGrid - 1)
//		{
//			MazePt next(curr.x + 1, curr.y + 1);
//			if (!m_visited[next.y * m_noGrid + next.x])
//			{
//				if (m_maze.Move(Maze::DIR_RIGHTUP) == true)
//				{
//					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_EMPTY;
//					DFS(next);
//					m_maze.Move(Maze::DIR_LEFTDOWN);
//				}
//				else
//				{
//					m_myGrid[next.y * m_noGrid + next.x] = Maze::TILE_WALL;
//				}
//			}
//		}
//	}
//}
//
//bool SceneHex::BFS(MazePt start, MazePt end)
//{
//	std::fill(m_visited.begin(), m_visited.end(), false);
//	while (!m_queue.empty())
//		m_queue.pop();
//	m_shortestPath.clear();
//	m_queue.push(start);
//	m_maze.SetNumMove(0);
//	while (!m_queue.empty())
//	{
//		MazePt curr = m_queue.front();
//		m_maze.SetCurr(curr);
//		m_queue.pop();
//		if (curr.x == end.x && curr.y == end.y)
//		{
//			while (!(curr.x == start.x && curr.y == start.y))
//			{
//				m_shortestPath.insert(m_shortestPath.begin(), curr);
//				curr = m_previous[curr.y * m_noGrid + curr.x];
//			}
//			m_shortestPath.insert(m_shortestPath.begin(), curr);
//			return true;
//		}
//		//UP
//		if (curr.y < m_noGrid - 1)
//		{
//			MazePt next(curr.x, curr.y + 1);
//			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
//			{
//				m_previous[next.y * m_noGrid + next.x] = curr;
//				m_queue.push(next);
//				m_visited[next.y * m_noGrid + next.x] = true;
//			}
//		}
//		//DOWN
//		if (curr.y > 0)
//		{
//			MazePt next(curr.x, curr.y - 1);
//			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
//			{
//				m_previous[next.y * m_noGrid + next.x] = curr;
//				m_queue.push(next);
//				m_visited[next.y * m_noGrid + next.x] = true;
//			}
//		}
//		//LEFT
//		if (curr.x > 0)
//		{
//			MazePt next(curr.x - 1, curr.y);
//			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
//			{
//				m_previous[next.y * m_noGrid + next.x] = curr;
//				m_queue.push(next);
//				m_visited[next.y * m_noGrid + next.x] = true;
//			}
//		}
//		//RIGHT
//		if (curr.x < m_noGrid - 1)
//		{
//			MazePt next(curr.x + 1, curr.y);
//			if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
//			{
//				m_previous[next.y * m_noGrid + next.x] = curr;
//				m_queue.push(next);
//				m_visited[next.y * m_noGrid + next.x] = true;
//			}
//		}
//		//LEFT ALT (Inside Left)
//		if (curr.x % 2 == 0) // Even
//		{
//			if (curr.x > 0 && curr.y > 0)
//			{
//				MazePt next(curr.x - 1, curr.y - 1);
//				if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
//				{
//					m_previous[next.y * m_noGrid + next.x] = curr;
//					m_queue.push(next);
//					m_visited[next.y * m_noGrid + next.x] = true;
//				}
//			}
//		}
//		else // Odd
//		{
//			if (curr.x > 0 && curr.y < m_noGrid - 1)
//			{
//				MazePt next(curr.x - 1, curr.y + 1);
//				if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
//				{
//					m_previous[next.y * m_noGrid + next.x] = curr;
//					m_queue.push(next);
//					m_visited[next.y * m_noGrid + next.x] = true;
//				}
//			}
//		}
//		//RIGHT ALT (Inside Right)
//		if (curr.x % 2 == 0) // Even
//		{
//			if (curr.x < m_noGrid - 1 && curr.y > 0)
//			{
//				MazePt next(curr.x + 1, curr.y - 1);
//				if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
//				{
//					m_previous[next.y * m_noGrid + next.x] = curr;
//					m_queue.push(next);
//					m_visited[next.y * m_noGrid + next.x] = true;
//				}
//			}
//		}
//		else // Odd
//		{
//			if (curr.x < m_noGrid - 1 && curr.y < m_noGrid - 1)
//			{
//				MazePt next(curr.x + 1, curr.y + 1);
//				if (!m_visited[next.y * m_noGrid + next.x] && m_myGrid[next.y * m_noGrid + next.x] == Maze::TILE_EMPTY)
//				{
//					m_previous[next.y * m_noGrid + next.x] = curr;
//					m_queue.push(next);
//					m_visited[next.y * m_noGrid + next.x] = true;
//				}
//			}
//		}
//	}
//	return false;
//}
//
//void SceneHex::Update(double dt)
//{
//	SceneBase::Update( dt );
//
//	//Calculating aspect ratio
//	m_worldHeight = 100.f;
//	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
//
//	if (Application::IsKeyPressed(VK_OEM_MINUS))
//	{
//		m_speed = Math::Max(0.f, m_speed - 0.1f);
//	}
//	if (Application::IsKeyPressed(VK_OEM_PLUS))
//	{
//		m_speed += 0.1f;
//	}
//	if ( Application::IsKeyPressed('N'))
//	{
//	}
//	if (Application::IsKeyPressed('M'))
//	{
//	}
//	if (Application::IsKeyPressed(VK_RETURN))
//	{
//	}
//	if (Application::IsKeyPressed('R'))
//	{
//		//Exercise: Implement Reset button
//	}
//
//	//Input Section
//	static bool bLButtonState = false;
//	if (!bLButtonState && Application::IsMousePressed(0))
//	{
//		bLButtonState = true;
//		std::cout << "LBUTTON DOWN" << std::endl;
//		double x, y;
//		Application::GetCursorPos(&x, &y);
//		int w = Application::GetWindowWidth();
//		int h = Application::GetWindowHeight();
//		float posX = static_cast<float>(x) / w * m_worldWidth;
//		float posY = (h - static_cast<float>(y)) / h * m_worldHeight;
//
//		float width = (m_noGrid * 0.75f + 0.25f) * m_gridSize;
//		float height = 0.866f * (m_noGrid + 0.5f) * m_gridSize;
//
//		if (posX < width && posY < height)
//		{
//			m_end.x = (int)(posX / width * m_noGrid);
//			if (m_end.x % 2 == 0)
//				m_end.y = Math::Min(m_noGrid - 1, (int)(posY / (m_gridSize * 0.866f)));
//			else
//				m_end.y = Math::Max(0, (int)((posY - 0.433f * m_gridSize) / (m_gridSize * 0.866f)));
//
//			BFS(m_start, m_end);
//		}
//
//	/*	if (posX < m_noGrid * m_gridSize && posY < m_noGrid * m_gridSize)
//		{
//			m_end.Set((int)(posX / m_gridSize), (int)(posY / m_gridSize));
//			BFS(m_start, m_end);
//			std::cout << "Path:";
//			for (auto tile : m_shortestPath)
//			{
//				std::cout << "(" << tile.x << "," << tile.y << ")";
//			}
//			std::cout << std::endl;
//		}*/
//	}
//	else if (bLButtonState && !Application::IsMousePressed(0))
//	{
//		bLButtonState = false;
//		std::cout << "LBUTTON UP" << std::endl;
//	}
//	static bool bRButtonState = false;
//	if (!bRButtonState && Application::IsMousePressed(1))
//	{
//		bRButtonState = true;
//		std::cout << "RBUTTON DOWN" << std::endl;
//	}
//	else if (bRButtonState && !Application::IsMousePressed(1))
//	{
//		bRButtonState = false;
//		std::cout << "RBUTTON UP" << std::endl;
//	}
//	static bool bSpaceState = false;
//	if (!bSpaceState && Application::IsKeyPressed(VK_SPACE))
//	{
//		bSpaceState = true;
//	}
//	else if (bSpaceState && !Application::IsKeyPressed(VK_SPACE))
//	{
//		bSpaceState = false;
//	}
//}
//
//void SceneHex::RenderGO(GameObject *go)
//{
//	switch (go->type)
//	{
//	case GameObject::NONE:
//		break;
//	}
//}
//
//void SceneHex::Render()
//{
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	// Projection matrix : Orthographic Projection
//	Mtx44 projection;
//	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
//	projectionStack.LoadMatrix(projection);
//
//	// Camera matrix
//	viewStack.LoadIdentity();
//	viewStack.LookAt(
//		camera.position.x, camera.position.y, camera.position.z,
//		camera.target.x, camera.target.y, camera.target.z,
//		camera.up.x, camera.up.y, camera.up.z
//		);
//	// Model matrix : an identity matrix (model will be at the origin)
//	modelStack.LoadIdentity();
//
//	RenderMesh(meshList[GEO_AXES], false);
//
//	modelStack.PushMatrix();
//	modelStack.Translate(m_worldHeight * 0.5f, m_worldHeight * 0.5f, -1.f);
//	modelStack.Scale(m_worldHeight, m_worldHeight, m_worldHeight);
//	RenderMesh(meshList[GEO_WHITEQUAD], false);
//	modelStack.PopMatrix();
//
//	//Render tiles 
//	//for (int row = 0; row < m_noGrid; ++row)
//	//{
//	//	for (int col = 0; col < m_noGrid; ++col)
//	//	{
//	//		modelStack.PushMatrix();
//	//		modelStack.Translate(m_gridOffset + m_gridSize * col, m_gridOffset + m_gridSize * row, 0);
//	//		modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
//	//		switch (m_myGrid[row * m_noGrid + col])
//	//		{
//	//		case Maze::TILE_WALL:
//	//			RenderMesh(meshList[GEO_GREYQUAD], false);
//	//			break;
//	//		case Maze::TILE_FOG:
//	//			RenderMesh(meshList[GEO_BLACKQUAD], false);
//	//			break;
//	//		case Maze::TILE_EMPTY:
//	//			RenderMesh(meshList[GEO_WHITEQUAD], false);
//	//			break;
//	//		}
//	//		modelStack.PopMatrix();
//	//	}
//	//}
//
//	// Rendering Hex grid
//	float worldX, worldY;
//	float xOffset, yOffset;
//	
//	xOffset = m_gridSize * 0.75f;
//
//	float len = sinf(Math::DegreeToRadian( 60.f )) * (2.f * m_gridSize);
//	yOffset = len / 2.f;
//
//	for (int x = 0; x < m_noGrid; ++x)
//	{
//		for (int y = 0; y < m_noGrid; ++y)
//		{
//			worldX = (x * xOffset) + m_gridOffset;
//			worldY = (y * yOffset) + m_gridOffset;
//
//			if (x % 2 != 0)
//				worldY += (yOffset / 2.f);
//
//			int currIDX = (y * m_noGrid) + x;
//
//			modelStack.PushMatrix();
//				modelStack.Translate(worldX, worldY, 0.0f);
//				modelStack.Rotate(90.0f, 1.f, 0.f, 0.f);
//				modelStack.Scale(m_gridSize, m_gridSize, m_gridSize);
//				switch ( m_myGrid[currIDX] )
//				{
//					case Maze::TILE_EMPTY: RenderMesh(meshList[GEO_WHITE_HEX], false); break;
//					case Maze::TILE_WALL:  RenderMesh(meshList[GEO_GREY_HEX], false); break;
//					case Maze::TILE_FOG:   RenderMesh(meshList[GEO_BLACK_HEX], false); break;
//				}
//			modelStack.PopMatrix();
//		}
//	}
//
//	//Render curr point
//	MazePt curr = m_maze.GetCurr();
//	worldX = (curr.x * xOffset) + m_gridOffset;
//	worldY = (curr.y * yOffset) + m_gridOffset;
//
//	if (curr.x % 2 != 0)
//		worldY += (yOffset / 2.f);
//
//	modelStack.PushMatrix();
//		modelStack.Translate(worldX, worldY, 0.01f);
//		modelStack.Scale(m_gridSize * 0.73f, m_gridSize * 0.73f, m_gridSize);
//		RenderMesh(meshList[GEO_QUEEN], false);
//	modelStack.PopMatrix();
//	//Render shortest path
//
//	for (auto tile : m_shortestPath)
//	{
//		worldX = (tile.x * xOffset) + m_gridOffset;
//		worldY = (tile.y * yOffset) + m_gridOffset;
//
//		if (tile.x % 2 != 0)
//			worldY += (yOffset / 2.f);
//
//		int currIDX = (tile.y * m_gridSize) + tile.x;
//
//		modelStack.PushMatrix();
//			modelStack.Translate(worldX, worldY, 0.01f);
//			modelStack.Scale(m_gridSize * 0.73f, m_gridSize * 0.73f, m_gridSize);
//			RenderMesh(meshList[GEO_QUEEN], false);
//		modelStack.PopMatrix();
//	}
//
//	//On screen text
//	std::ostringstream ss;
//	ss.precision(3);
//	ss << "Speed:" << m_speed;
//	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 6);
//
//	ss.str("");
//	ss.precision(5);
//	ss << "FPS:" << fps;
//	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 3);
//
//	ss.str("");
//	ss << "Num Move:" << m_maze.GetNumMove();
//	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 9);
//
//	ss.str("");
//	ss << "Turn Maze " << m_mazeKey;
//	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 50, 0);
//}
//
//void SceneHex::Exit()
//{
//	SceneBase::Exit();
//	//Cleanup GameObjects
//	while (m_goList.size() > 0)
//	{
//		GameObject *go = m_goList.back();
//		delete go;
//		m_goList.pop_back();
//	}
//}
