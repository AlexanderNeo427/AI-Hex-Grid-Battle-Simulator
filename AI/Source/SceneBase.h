#ifndef SCENE_BASE_H
#define SCENE_BASE_H

#include "Scene.h"
#include "Mtx44.h"
#include "Camera.h"
#include "Mesh.h"
#include "MatrixStack.h"
#include "Light.h"
#include <vector>

class SceneBase : public Scene
{
	enum UNIFORM_TYPE
	{
		U_MVP = 0,
		U_MODELVIEW,
		U_MODELVIEW_INVERSE_TRANSPOSE,
		U_MATERIAL_AMBIENT,
		U_MATERIAL_DIFFUSE,
		U_MATERIAL_SPECULAR,
		U_MATERIAL_SHININESS,
		U_LIGHTENABLED,
		U_NUMLIGHTS,
		U_LIGHT0_TYPE,
		U_LIGHT0_POSITION,
		U_LIGHT0_COLOR,
		U_LIGHT0_POWER,
		U_LIGHT0_KC,
		U_LIGHT0_KL,
		U_LIGHT0_KQ,
		U_LIGHT0_SPOTDIRECTION,
		U_LIGHT0_COSCUTOFF,
		U_LIGHT0_COSINNER,
		U_LIGHT0_EXPONENT,
		U_COLOR_TEXTURE_ENABLED,
		U_COLOR_TEXTURE,
		U_TEXT_ENABLED,
		U_TEXT_COLOR,
		U_TOTAL,
	};
public:
	enum GEOMETRY_TYPE
	{
		GEO_AXES,
		GEO_TEXT,
		GEO_BALL,
		GEO_CUBE,
		GEO_BG,
		GEO_TICTACTOE,
		GEO_CROSS,
		GEO_CIRCLE,
		GEO_BLUE_LINE,
		GEO_RED_LINE,

		GEO_DEAD, 
		GEO_FISHFOOD, 
		GEO_FULL, 
		GEO_HUNGRY, 
		GEO_TOOFULL,
		GEO_CRAZY, 
		GEO_NAUGHTY, 
		GEO_HAPPY,

		GEO_BG2,
		GEO_ATTACK_SPAWN,
		GEO_DEFEND_SPAWN,
		GEO_TOWER,
		GEO_ORB,

		GEO_RED_ORB,
		GEO_BLUE_ORB,

		GEO_COVER,
		GEO_ATTACK_HP,
		GEO_DEFEND_HP,

		GEO_WALL_UNBROKEN,
		GEO_WALL_SLIGHTLY_BROKEN,
		GEO_WALL_VERY_BROKEN,

		// Assignment 2
		GEO_SOLDIER_ATTACKER,
		GEO_SOLDIER_DEFENDER,

		GEO_ARCHER_ATTACKER,
		GEO_ARCHER_DEFENDER,

		GEO_MEDIC_ATTACKER,
		GEO_MEDIC_DEFENDER,

		GEO_TANK_ATTACKER,
		GEO_TANK_DEFENDER,
		
		GEO_CHESSBOARD,
		GEO_QUEEN,
		GEO_KNIGHT,

		GEO_WHITEQUAD,	// Empty
		GEO_GREYQUAD,	// Wall
		GEO_BLACKQUAD,	// Uncharted
		GEO_YELLOWQUAD,	// Visited

		GEO_BLACK_HEX,	// Fog, undiscovered : 0
		GEO_GREY_HEX,	// Wall, non-traversable : - 1
		GEO_WHITE_HEX,	// Empty : 1
		GEO_GREEN_HEX,	// Grass : 2
		GEO_BLUE_HEX,	// Water : 3
		GEO_BROWN_HEX,  // Mud   : 4
		GEO_RED_HEX,	// Explosion

		GEO_GREY_HEX_MUTED,	 // Wall, non-traversable : - 1
		GEO_WHITE_HEX_MUTED, // Empty : 1
		GEO_GREEN_HEX_MUTED, // Grass : 2
		GEO_BLUE_HEX_MUTED,	 // Water : 3
		GEO_BROWN_HEX_MUTED, // Mud   : 4

		GEO_ARROW_RED,
		GEO_ARROW_BLUE,

		GEO_GRENADE_RED,
		GEO_GRENADE_BLUE,

		NUM_GEOMETRY,
	};
public:
	SceneBase();
	~SceneBase();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderText(Mesh* mesh, std::string text, Color color);
	void RenderTextOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y);
	void RenderMesh(Mesh *mesh, bool enableLight);
protected:
	unsigned m_vertexArrayID;
	Mesh* meshList[NUM_GEOMETRY];
	unsigned m_programID;
	unsigned m_parameters[U_TOTAL];

	Camera camera;

	MS modelStack;
	MS viewStack;
	MS projectionStack;

	Light lights[1];

	bool bLightEnabled;

	float fps;
};

#endif