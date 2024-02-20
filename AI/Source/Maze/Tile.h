#ifndef _TILE_H_
#define _TILE_H_

#include "MazePt.h"
#include "Vector3.h"

#include <map>

class Tile
{
public:
	enum class CONTENT
	{
		WATER,
		WALL, 
		EMPTY,
		GRASS,
		MUD,

		MAX,
	};
public:
	Tile(CONTENT _content, MazePt _mazePt, Vector3 _worldPt);
	~Tile();

	int GetCost() const;

	CONTENT GetContent() const;
	void SetContent(CONTENT _tileContent);

	MazePt GetMazePt() const;
	Vector3 GetWorldPt() const;

	bool GetIsVisited() const;
	void SetIsVisited(bool _isVisited);

	float GetGlobalCost() const;
	void SetGlobalCost(float _gCost);

	float GetHeuristic() const;
	void SetHeuristic(float _hCost);

	float GetFinalCost() const;
	float CalcFinalCost() const;
	void SetFinalCost(float _fCost);
private:
	CONTENT m_tileContent;
	MazePt  m_mazePt;
	Vector3 m_worldPt; // Caching
	bool    m_isVisited;

	float   m_globalCost;
	float   m_heuristicCost;
	float   m_finalCost;

	static std::map<Tile::CONTENT, int> m_tileCostMap;
};

#endif

