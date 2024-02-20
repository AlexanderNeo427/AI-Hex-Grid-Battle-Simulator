#include "SceneData.h"
#include "Application.h"

Vector3 SceneData::GetWorldSize() const { return m_worldSize; }
float SceneData::GetWorldWidth() const { return m_worldSize.x; }
float SceneData::GetWorldHeight() const { return m_worldSize.y; }
void SceneData::SetWorldSize(const Vector3 &_worldSize) { m_worldSize = _worldSize; }
void SceneData::SetWorldSize(int _worldWidth, int _worldHeight)
{
	m_worldSize.x = _worldWidth;
	m_worldSize.y = _worldHeight;
}

int SceneData::GetNumGrid() const { return m_noGrid; }
void SceneData::SetNumGrid(int _numGrid) { m_noGrid = _numGrid; }

float SceneData::GetGridOffset() const { return m_gridOffset; }
void SceneData::SetGridOffset(float _gridOffset) { m_gridOffset = _gridOffset; }

float SceneData::GetGridSize() const { return m_gridSize; }
void SceneData::SetGridSize(float _gridSize) { m_gridSize = _gridSize; }

Maze* SceneData::GetWorldMaze() const { return m_maze; }
void SceneData::SetWorldMaze(Maze* _pMaze) { m_maze = _pMaze; }

SceneData::SceneData() {}
SceneData::~SceneData() {}

namespace Helper
{
	static SceneData& data = *SceneData::GetInstance();

	Vector3 GetMousePos() 
	{
		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * data.GetWorldSize().x;
		float posY = (h - static_cast<float>(y)) / h * data.GetWorldSize().y;

		return Vector3(posX, posY, 0.001f);
	}
	MazePt GetMouseGrid2D()
	{
		double x, y;
		Application::GetCursorPos(&x, &y);
		int w = Application::GetWindowWidth();
		int h = Application::GetWindowHeight();
		float posX = static_cast<float>(x) / w * data.GetWorldSize().x;
		float posY = (h - static_cast<float>(y)) / h * data.GetWorldSize().y;

		(float)data.GetNumGrid();
		data.GetGridSize();

		float width = ((float)data.GetNumGrid() * 0.75f + 0.25f) * data.GetGridSize();
		float height = 0.866f * ((float)data.GetNumGrid() + 0.5f) * data.GetGridSize();

		Vector3 mazePt;

		if (posX < width && posY < height)
		{
			mazePt.x = (int)(posX / width * (float)data.GetNumGrid());
			if ((int)mazePt.x % 2 == 0)
				mazePt.y = Math::Min(data.GetNumGrid() - 1, (int)(posY / (data.GetGridSize() * 0.866f)));
			else
				mazePt.y = Math::Max(0, (int)((posY - 0.433f * data.GetGridSize()) / (data.GetGridSize() * 0.866f)));
		}

		return MazePt(static_cast<int>(mazePt.x), static_cast<int>(mazePt.y));
	}
}

namespace Transform
{
	static SceneData& data = *SceneData::GetInstance();

	Vector3 MazeToWorldPt(const MazePt& mazePt)
	{
		float worldX, worldY;
		float xOffset, yOffset;

		xOffset = data.GetGridSize() * 0.75f;

		float len = sinf(Math::DegreeToRadian(60.f)) * (2.0f * data.GetGridSize());
		yOffset = len / 2.0f;

		worldX = (mazePt.x * xOffset) + data.GetGridOffset();
		worldY = (mazePt.y * yOffset) + data.GetGridOffset();

		if (mazePt.x % 2 != 0)
			worldY += (yOffset / 2.f);

		return Vector3(worldX, worldY, 0.001f);
	}

	int Grid2Dto1D(const MazePt& mazePt) { return mazePt.y * data.GetNumGrid() + mazePt.x; }
	int Grid2Dto1D(int x, int y) { return y * data.GetNumGrid() + x; }
}


