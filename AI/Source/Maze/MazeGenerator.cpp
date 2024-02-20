#include "MazeGenerator.h"
#include "../SceneData.h"
#include "PerlinNoise2D.h"

static SceneData& data = *SceneData::GetInstance();

MazeGenerator::MazeGenerator()
{
}

MazeGenerator::~MazeGenerator()
{
}

void MazeGenerator::SimpleGenerate(unsigned key, Maze* pMaze)
{
	srand( key );
	pMaze->Reset( Tile::CONTENT::EMPTY );

	for (Tile* &tile : pMaze->GetGrid())
	{
		int max = static_cast<int>( Tile::CONTENT::MAX );
		int rand = Math::RandIntMinMax(0, max - 1);

		Tile::CONTENT randContent = static_cast<Tile::CONTENT>( rand );
		tile->SetContent( randContent );
	}
}

void MazeGenerator::MrTangsAlgorithm(unsigned key, MazePt start, float wallLoad, Maze* pMaze)
{
	pMaze->Reset( Tile::CONTENT::EMPTY );

	int startX = start.x;
	int startY = start.y;
	startX = Math::Clamp(startX, 0, data.GetNumGrid() - 1);
	startY = Math::Clamp(startY, 0, data.GetNumGrid() - 1);

	int total = data.GetNumGrid() * data.GetNumGrid();
	wallLoad = Math::Clamp(wallLoad, 0.f, 0.8f);
	int startIDX = (startY * data.GetNumGrid()) + startX;

	srand( key );
	int max_itr = total * wallLoad;

	for (int i = 0; i < max_itr;)
	{
		unsigned chosen = rand() % total;

		if (chosen == startIDX || chosen == startIDX + 1)
			continue;

		if (pMaze->See( chosen ) == Tile::CONTENT::EMPTY)
		{
			int max = static_cast<int>( Tile::CONTENT::MAX );
			int rand = Math::RandIntMinMax(0, max - 1);

			Tile::CONTENT randContent = static_cast<Tile::CONTENT>( rand );
			pMaze->GetGrid()[chosen]->SetContent( randContent );
			// maze->GetTile( chosen )->SetContent( Tile::CONTENT::WALL ); 
			++i;
		}
	}
}

void MazeGenerator::PerlinGenerate(int numOctaves, double persistance, Maze* pMaze)
{
	pMaze->Reset( Tile::CONTENT::EMPTY );

	int worldSize = static_cast<int>(data.GetWorldHeight());
	int size = worldSize * worldSize;
	double* mapArr = new double[size];

	double min = DBL_MAX;
	double max = DBL_MIN;

	PerlinNoise2D noise(data.GetWorldHeight(), data.GetWorldHeight(), numOctaves, persistance);

	for (int x = 0; x < worldSize; ++x)
	{
		for (int y = 0; y < worldSize; ++y)
		{
			double val = noise.ValueNoise_2D(x, y);
			mapArr[y * worldSize + x] = val;

			if (val < min)
				min = val;
			else if (val > max)
				max = val;
		}
	}

	double minOffset = -min;
	min = DBL_MAX;
	max = DBL_MIN;

	for (int i = 0; i < size; ++i)
	{
		mapArr[i] += minOffset;

		if (mapArr[i] < 0.0) 
			mapArr[i] = 0.0;

		if (mapArr[i] < min)
			min = mapArr[i];
		else if (mapArr[i] > max)
			max = mapArr[i];
	}

	double maxOffset = 1.0 / max;
	for (int i = 0; i < size; ++i)
	{
		mapArr[i] *= maxOffset;

		if (mapArr[i] < min)
			min = mapArr[i];
		else if (mapArr[i] > max)
			max = mapArr[i];
	}

	int worldX, worldY;
	for (int tileX = 0; tileX < data.GetNumGrid(); ++tileX)
	{
		for (int tileY = 0; tileY < data.GetNumGrid(); ++tileY)
		{
			Vector3 worldPt = Transform::MazeToWorldPt(MazePt(tileX, tileY));
			int idx = worldPt.y * worldSize + worldPt.x;
			double val = mapArr[idx];

			if (val >= 0.0 && val < 0.38)
			{
				pMaze->GetTile(tileX, tileY)->SetContent(Tile::CONTENT::WALL);
			}
			else if (val >= 0.38 && val < 0.42)
			{
				pMaze->GetTile(tileX, tileY)->SetContent(Tile::CONTENT::WATER);
			}
			else if (val >= 0.42 && val < 0.58)
			{
				pMaze->GetTile(tileX, tileY)->SetContent(Tile::CONTENT::EMPTY);
			}
			else if (val >= 0.58 && val < 0.7)
			{
				pMaze->GetTile(tileX, tileY)->SetContent(Tile::CONTENT::GRASS);
			}
			else if (val >= 0.7)
			{
				pMaze->GetTile(tileX, tileY)->SetContent(Tile::CONTENT::MUD);
			}
		}
	}

	pMaze->GetTile(0, 0)->SetContent( Tile::CONTENT::EMPTY );
	pMaze->GetTile(1, 0)->SetContent( Tile::CONTENT::EMPTY );
	pMaze->GetTile(0, 1)->SetContent( Tile::CONTENT::EMPTY );
	 
	int tmp = data.GetNumGrid() - 1;
	pMaze->GetTile(tmp, tmp)->SetContent( Tile::CONTENT::EMPTY );
	pMaze->GetTile(tmp - 1, tmp)->SetContent( Tile::CONTENT::EMPTY );
	pMaze->GetTile(tmp, tmp - 1)->SetContent( Tile::CONTENT::EMPTY );

	for (Tile* pTile : pMaze->GetGrid())
	{
		if (pTile->GetCost() != 1)
		{
			float clearChance = Math::RandFloatMinMax(0.f, 100.f);
			if (clearChance <= 18.5f)
				pTile->SetContent( Tile::CONTENT::EMPTY );
		}
	}
}




