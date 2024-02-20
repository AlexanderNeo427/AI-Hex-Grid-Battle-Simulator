#include "Tile.h"
#include "../SceneData.h"

static SceneData& data = *SceneData::GetInstance();

/*
*  Tile costs are defined here
* 
*  < 1 - Unpathable
*    0 - Cannot see past
*  > 1 - Pathable, but incurs move cost
*/
std::map<Tile::CONTENT, int> Tile::m_tileCostMap =
{
	{ CONTENT::WATER, -2 }, 
	{ CONTENT::WALL,   0 },	
	{ CONTENT::EMPTY,  1 },
	{ CONTENT::GRASS,  2 },
	{ CONTENT::MUD,    3 },
};    

Tile::Tile(CONTENT _content, MazePt _mazePt, Vector3 _worldPt)
	:
	m_tileContent( _content ),
	m_mazePt( _mazePt ),
	m_worldPt( _worldPt ),
	m_isVisited( false ),
	m_globalCost( 9999999.f ),
	m_heuristicCost( 9999999.f ),
	m_finalCost( 9999999.f )
{}

Tile::~Tile()
{
}

int Tile::GetCost() const { return m_tileCostMap[ m_tileContent ]; }

Tile::CONTENT Tile::GetContent() const { return m_tileContent; }

void Tile::SetContent(CONTENT _tileContent) { m_tileContent = _tileContent; }

MazePt Tile::GetMazePt() const { return m_mazePt; }

Vector3 Tile::GetWorldPt() const { return m_worldPt; }

bool Tile::GetIsVisited() const			 { return m_isVisited; }
void Tile::SetIsVisited(bool _isVisited) { m_isVisited = _isVisited; }

float Tile::GetGlobalCost() const	   { return m_globalCost; }
void Tile::SetGlobalCost(float _gCost) { m_globalCost = _gCost; }

float Tile::GetHeuristic() const	  { return m_heuristicCost; }
void Tile::SetHeuristic(float _hCost) { m_heuristicCost = _hCost; }

float Tile::GetFinalCost() const	  { return m_finalCost; }
float Tile::CalcFinalCost() const	  { return m_globalCost + m_heuristicCost; }
void Tile::SetFinalCost(float _fCost) { m_finalCost = _fCost; }






