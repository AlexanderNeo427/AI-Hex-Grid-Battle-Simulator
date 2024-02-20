#include "SceneBase.h"
#include "GL\glew.h"

#include "shader.hpp"
#include "MeshBuilder.h"
#include "Application.h"
#include "Utility.h"
#include "LoadTGA.h"
#include <sstream>

static const int fontWidth[] = { 0,26,26,26,26,26,26,26,26,26,26,26,26,0,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,12,17,21,26,26,37,35,11,16,16,26,26,13,16,13,20,26,26,26,26,26,26,26,26,26,26,14,14,26,26,26,24,46,30,28,28,32,25,24,33,32,13,17,27,22,44,34,34,27,35,28,24,25,33,30,46,27,25,24,16,20,16,26,26,15,25,27,22,27,26,16,24,27,12,12,24,12,42,27,27,27,27,18,20,17,27,23,37,23,24,21,16,24,16,26,26,26,26,13,16,22,36,26,26,21,54,24,18,45,26,24,26,26,13,13,22,22,26,26,47,23,37,20,18,44,26,21,25,12,17,26,26,26,26,26,26,20,43,21,27,26,16,26,20,18,26,17,17,15,29,30,13,16,13,22,27,33,35,35,24,30,30,30,30,30,30,40,28,25,25,25,25,13,13,13,13,32,34,34,34,34,34,34,26,35,33,33,33,33,25,27,27,25,25,25,25,25,25,40,22,26,26,26,26,12,12,12,12,27,27,27,27,27,27,27,26,28,27,27,27,27,24,27,24 };

SceneBase::SceneBase()
{
}

SceneBase::~SceneBase()
{
}

void SceneBase::Init()
{
	// Black background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
	
	glEnable(GL_CULL_FACE);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);

	m_programID = LoadShaders( "Shader//comg.vertexshader", "Shader//comg.fragmentshader" );
	
	// Get a handle for our uniform
	m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");
	//m_parameters[U_MODEL] = glGetUniformLocation(m_programID, "M");
	//m_parameters[U_VIEW] = glGetUniformLocation(m_programID, "V");
	m_parameters[U_MODELVIEW] = glGetUniformLocation(m_programID, "MV");
	m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE] = glGetUniformLocation(m_programID, "MV_inverse_transpose");
	m_parameters[U_MATERIAL_AMBIENT] = glGetUniformLocation(m_programID, "material.kAmbient");
	m_parameters[U_MATERIAL_DIFFUSE] = glGetUniformLocation(m_programID, "material.kDiffuse");
	m_parameters[U_MATERIAL_SPECULAR] = glGetUniformLocation(m_programID, "material.kSpecular");
	m_parameters[U_MATERIAL_SHININESS] = glGetUniformLocation(m_programID, "material.kShininess");
	m_parameters[U_LIGHTENABLED] = glGetUniformLocation(m_programID, "lightEnabled");
	m_parameters[U_NUMLIGHTS] = glGetUniformLocation(m_programID, "numLights");
	m_parameters[U_LIGHT0_TYPE] = glGetUniformLocation(m_programID, "lights[0].type");
	m_parameters[U_LIGHT0_POSITION] = glGetUniformLocation(m_programID, "lights[0].position_cameraspace");
	m_parameters[U_LIGHT0_COLOR] = glGetUniformLocation(m_programID, "lights[0].color");
	m_parameters[U_LIGHT0_POWER] = glGetUniformLocation(m_programID, "lights[0].power");
	m_parameters[U_LIGHT0_KC] = glGetUniformLocation(m_programID, "lights[0].kC");
	m_parameters[U_LIGHT0_KL] = glGetUniformLocation(m_programID, "lights[0].kL");
	m_parameters[U_LIGHT0_KQ] = glGetUniformLocation(m_programID, "lights[0].kQ");
	m_parameters[U_LIGHT0_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[0].spotDirection");
	m_parameters[U_LIGHT0_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[0].cosCutoff");
	m_parameters[U_LIGHT0_COSINNER] = glGetUniformLocation(m_programID, "lights[0].cosInner");
	m_parameters[U_LIGHT0_EXPONENT] = glGetUniformLocation(m_programID, "lights[0].exponent");
	// Get a handle for our "colorTexture" uniform
	m_parameters[U_COLOR_TEXTURE_ENABLED] = glGetUniformLocation(m_programID, "colorTextureEnabled");
	m_parameters[U_COLOR_TEXTURE] = glGetUniformLocation(m_programID, "colorTexture");
	// Get a handle for our "textColor" uniform
	m_parameters[U_TEXT_ENABLED] = glGetUniformLocation(m_programID, "textEnabled");
	m_parameters[U_TEXT_COLOR] = glGetUniformLocation(m_programID, "textColor");
	
	// Use our shader
	glUseProgram(m_programID);

	lights[0].type = Light::LIGHT_DIRECTIONAL;
	lights[0].position.Set(0, 20, 0);
	lights[0].color.Set(1, 1, 1);
	lights[0].power = 1;
	lights[0].kC = 1.f;
	lights[0].kL = 0.01f;
	lights[0].kQ = 0.001f;
	lights[0].cosCutoff = cos(Math::DegreeToRadian(45));
	lights[0].cosInner = cos(Math::DegreeToRadian(30));
	lights[0].exponent = 3.f;
	lights[0].spotDirection.Set(0.f, 1.f, 0.f);
	
	glUniform1i(m_parameters[U_NUMLIGHTS], 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);

	glUniform1i(m_parameters[U_LIGHT0_TYPE], lights[0].type);
	glUniform3fv(m_parameters[U_LIGHT0_COLOR], 1, &lights[0].color.r);
	glUniform1f(m_parameters[U_LIGHT0_POWER], lights[0].power);
	glUniform1f(m_parameters[U_LIGHT0_KC], lights[0].kC);
	glUniform1f(m_parameters[U_LIGHT0_KL], lights[0].kL);
	glUniform1f(m_parameters[U_LIGHT0_KQ], lights[0].kQ);
	glUniform1f(m_parameters[U_LIGHT0_COSCUTOFF], lights[0].cosCutoff);
	glUniform1f(m_parameters[U_LIGHT0_COSINNER], lights[0].cosInner);
	glUniform1f(m_parameters[U_LIGHT0_EXPONENT], lights[0].exponent);

	camera.Init(Vector3(0, 0, 1), Vector3(0, 0, 0), Vector3(0, 1, 0));

	for(int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = NULL;
	}
	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("reference", 1000, 1000, 1000);
	meshList[GEO_BALL] = MeshBuilder::GenerateSphere("ball", Color(1, 0, 0), 10, 10, 0.5f);
	meshList[GEO_CUBE] = MeshBuilder::GenerateCube("cube", Color(1, 1, 1), 1.f);
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);

	meshList[GEO_TEXT]->textureID = LoadTGA("Image//calibri.tga");
	//meshList[GEO_BG] = MeshBuilder::GenerateQuad("bg", Color(1, 1, 1));
	//meshList[GEO_BG]->textureID = LoadTGA("Image//grid20.tga");
	//meshList[GEO_TICTACTOE] = MeshBuilder::GenerateQuad("tictactoe", Color(1, 1, 1));
	//meshList[GEO_TICTACTOE]->textureID = LoadTGA("Image//tictactoe.tga");
	//meshList[GEO_CROSS] = MeshBuilder::GenerateQuad("cross", Color(1, 1, 1));
	//meshList[GEO_CROSS]->textureID = LoadTGA("Image//cross.tga");
	//meshList[GEO_CIRCLE] = MeshBuilder::GenerateQuad("circle", Color(1, 1, 1));
	//meshList[GEO_CIRCLE]->textureID = LoadTGA("Image//circle.tga");
	//meshList[GEO_TEXT]->material.kAmbient.Set(1, 0, 0);
	//
	//meshList[GEO_DEAD] = MeshBuilder::GenerateQuad("dead", Color(1, 1, 1));
	//meshList[GEO_DEAD]->textureID = LoadTGA("Image//dead.tga");
	//meshList[GEO_FISHFOOD] = MeshBuilder::GenerateQuad("fishfood", Color(1, 1, 1));
	//meshList[GEO_FISHFOOD]->textureID = LoadTGA("Image//fishfood.tga");
	//meshList[GEO_FULL] = MeshBuilder::GenerateQuad("full", Color(1, 1, 1));
	//meshList[GEO_FULL]->textureID = LoadTGA("Image//full.tga");
	//meshList[GEO_HUNGRY] = MeshBuilder::GenerateQuad("hungry", Color(1, 1, 1));
	//meshList[GEO_HUNGRY]->textureID = LoadTGA("Image//hungry.tga");
	//meshList[GEO_TOOFULL] = MeshBuilder::GenerateQuad("toofull", Color(1, 1, 1));
	//meshList[GEO_TOOFULL]->textureID = LoadTGA("Image//toofull.tga");
	//
	//meshList[GEO_CRAZY] = MeshBuilder::GenerateQuad("SharkCrazy", Color(1, 1, 1));
	//meshList[GEO_CRAZY]->textureID = LoadTGA("Image//crazy.tga");
	//meshList[GEO_NAUGHTY] = MeshBuilder::GenerateQuad("SharkNaughty", Color(1, 1, 1));
	//meshList[GEO_NAUGHTY]->textureID = LoadTGA("Image//shark.tga");
	//meshList[GEO_HAPPY] = MeshBuilder::GenerateQuad("SharkHappy", Color(1, 1, 1));
	//meshList[GEO_HAPPY]->textureID = LoadTGA("Image//happy.tga");

	// Assignment 1
	meshList[GEO_BG2] = MeshBuilder::GenerateQuad("Background2", Color(1, 1, 1));
	meshList[GEO_BG2]->textureID = LoadTGA("Image//Grid45.tga");

	meshList[GEO_ATTACK_SPAWN] = MeshBuilder::GenerateCube("AttackSpawn", Color(0.75f, 0.18f, 0.18f), 1.f);
	meshList[GEO_DEFEND_SPAWN] = MeshBuilder::GenerateCube("DefendSpawn", Color(0.18f, 0.18f, 0.75f), 1.f);
	
	meshList[GEO_ATTACK_HP] = MeshBuilder::GenerateCube("AttackHP", Color(1.0f, 0.f, 0.f), 1.f);
	meshList[GEO_DEFEND_HP] = MeshBuilder::GenerateCube("DefendHP", Color(0.1f, 0.1f, 0.88f), 1.f);

	meshList[GEO_ORB] = MeshBuilder::GenerateSphere("Orb", Color(0.376f, 0.902f, 0.831f), 10, 10, 0.5f);
	meshList[GEO_RED_ORB] = MeshBuilder::GenerateSphere("RedOrb", Color(1.f, 0.f, 0.f), 10, 10, 0.5f);
	meshList[GEO_BLUE_ORB] = MeshBuilder::GenerateSphere("BlueOrb", Color(0.f, 0.f, 1.0f), 10, 10, 0.5f);

	meshList[GEO_TOWER] = MeshBuilder::GenerateSphere("Tower", Color(0.2f, 0.2f, 0.6f), 10, 10, 0.5f);
	meshList[GEO_COVER] = MeshBuilder::GenerateCube("Cover", Color(0.1f, 0.9f, 0.1f), 1.f);

	meshList[GEO_WALL_UNBROKEN] = MeshBuilder::GenerateCube("WallUnbroken", Color(0.16f, 0.08f, 0.02f), 1.f);
	meshList[GEO_WALL_SLIGHTLY_BROKEN] = MeshBuilder::GenerateCube("WallSlightlyBroken", Color(0.383f, 0.29f, 0.18f), 1.f);
	meshList[GEO_WALL_VERY_BROKEN] = MeshBuilder::GenerateCube("WallVeryBroken", Color(0.71f, 0.396f, 0.114f), 1.f);

	// Generate line
	meshList[GEO_RED_LINE] = MeshBuilder::GenerateLine("RedLine", Color(1, 0, 0), 1.f);
	meshList[GEO_BLUE_LINE] = MeshBuilder::GenerateLine("BlueLine", Color(0, 0, 1), 1.f);

	meshList[GEO_SOLDIER_ATTACKER] = MeshBuilder::GenerateQuad("SoldierAttacker", Color(1, 1, 1));
	meshList[GEO_SOLDIER_ATTACKER]->textureID = LoadTGA("Image//A2//SoldierAttacker.tga");
	meshList[GEO_ARCHER_ATTACKER] = MeshBuilder::GenerateQuad("ArcherAttacker", Color(1, 1, 1));
	meshList[GEO_ARCHER_ATTACKER]->textureID = LoadTGA("Image//A2//ArcherAttacker.tga");
	meshList[GEO_MEDIC_ATTACKER] = MeshBuilder::GenerateQuad("MedicAttacker", Color(1, 1, 1));
	meshList[GEO_MEDIC_ATTACKER]->textureID = LoadTGA("Image//A2//MedicAttacker.tga");
	meshList[GEO_TANK_ATTACKER] = MeshBuilder::GenerateQuad("TankAttacker", Color(1, 1, 1));
	meshList[GEO_TANK_ATTACKER]->textureID = LoadTGA("Image//A2//AerialAttacker.tga");

	meshList[GEO_SOLDIER_DEFENDER] = MeshBuilder::GenerateQuad("SoldierDefender", Color(1, 1, 1));
	meshList[GEO_SOLDIER_DEFENDER]->textureID = LoadTGA("Image//A2//SoldierDefender.tga");
	meshList[GEO_ARCHER_DEFENDER] = MeshBuilder::GenerateQuad("ArcherDefender", Color(1, 1, 1));
	meshList[GEO_ARCHER_DEFENDER]->textureID = LoadTGA("Image//A2//ArcherDefender.tga");
	meshList[GEO_MEDIC_DEFENDER] = MeshBuilder::GenerateQuad("MedicDefender", Color(1, 1, 1));
	meshList[GEO_MEDIC_DEFENDER]->textureID = LoadTGA("Image//A2//MedicDefender.tga");
	meshList[GEO_TANK_DEFENDER] = MeshBuilder::GenerateQuad("TankDefender", Color(1, 1, 1));
	meshList[GEO_TANK_DEFENDER]->textureID = LoadTGA("Image//A2//AerialDefender.tga");

	// Lab 6
	meshList[GEO_CHESSBOARD] = MeshBuilder::GenerateQuad("Chessboard", Color(1, 1, 1));
	meshList[GEO_CHESSBOARD]->textureID = LoadTGA("Image//chessboard.tga");

	meshList[GEO_QUEEN] = MeshBuilder::GenerateQuad("Queen", Color(1, 1, 1));
	meshList[GEO_QUEEN]->textureID = LoadTGA("Image//queen.tga");

	meshList[GEO_KNIGHT] = MeshBuilder::GenerateQuad("Knight", Color(1, 1, 1));
	meshList[GEO_KNIGHT]->textureID = LoadTGA("Image//knight.tga");

	// Lab 7
	meshList[GEO_WHITEQUAD] = MeshBuilder::GenerateCube("whiteCube", Color(1, 1, 1), 1.f);
	meshList[GEO_GREYQUAD] = MeshBuilder::GenerateCube("greyCube", Color(0.4f, 0.4f, 0.4f), 1.f);
	meshList[GEO_BLACKQUAD] = MeshBuilder::GenerateCube("blackCube", Color(0.f, 0.f, 0.f), 1.f);
	meshList[GEO_YELLOWQUAD] = MeshBuilder::GenerateCube("yellowCube", Color(1.f, 1.f, 0.f), 1.f);

	// Hex
	meshList[GEO_BLACK_HEX] = MeshBuilder::GenerateRing("blackHex", Color(0, 0, 0), 6, 0.5f, 0.0f);

	meshList[GEO_GREY_HEX]  = MeshBuilder::GenerateRing("greyHex", Color(0.4f, 0.4f, 0.2f), 6, 0.5f, 0.0f);
	meshList[GEO_GREY_HEX_MUTED]  = MeshBuilder::GenerateRing("greyHexMuted", Color(0.13f, 0.13f, 0.13f), 6, 0.5f, 0.0f);

	meshList[GEO_WHITE_HEX] = MeshBuilder::GenerateRing("whiteHex", Color(1, 1, 1), 6, 0.5f, 0.0f);
	meshList[GEO_WHITE_HEX_MUTED] = MeshBuilder::GenerateRing("whiteHexMuted", Color(0.3f, 0.3f, 0.3f), 6, 0.5f, 0.0f);

	meshList[GEO_GREEN_HEX] = MeshBuilder::GenerateRing("greenHex", Color(0.f, 1.0f, 0.0f), 6, 0.5f, 0.0f);
	meshList[GEO_GREEN_HEX_MUTED] = MeshBuilder::GenerateRing("greenHexMuted", Color(0.f, 0.3f, 0.0f), 6, 0.5f, 0.0f);

	meshList[GEO_BLUE_HEX]  = MeshBuilder::GenerateRing("blueHex", Color(0.2f, 0.875f, 1.f), 6, 0.5f, 0.0f);
	meshList[GEO_BLUE_HEX_MUTED]  = MeshBuilder::GenerateRing("blueHexMuted", Color(0.06f, 0.26f, 0.3f), 6, 0.5f, 0.0f);

	meshList[GEO_BROWN_HEX] = MeshBuilder::GenerateRing("brownHex", Color(0.55f, 0.28f, 0.1f), 6, 0.5f, 0.0f);
	meshList[GEO_BROWN_HEX_MUTED] = MeshBuilder::GenerateRing("brownHexMuted", Color(0.162f, 0.082f, 0.03f), 6, 0.5f, 0.0f);

	meshList[GEO_RED_HEX] = MeshBuilder::GenerateRing("redHex", Color(0.8f, 0.1f, 0.1f), 6, 0.5f, 0.0f);

	meshList[GEO_ARROW_RED] = MeshBuilder::GenerateCube("ArrowRed", Color(1.0f, 0.f, 0.f), 1.f);
	meshList[GEO_ARROW_BLUE] = MeshBuilder::GenerateCube("ArrowBlue", Color(0.0f, 0.0f, 1.0f), 1.f);

	meshList[GEO_GRENADE_RED] = MeshBuilder::GenerateSphere("GrenadeRed", Color(1.0, 0.0f, 0.0f), 10, 10, 0.5f);
	meshList[GEO_GRENADE_BLUE] = MeshBuilder::GenerateSphere("GrenadeBlue", Color(0.0f, 0.0f, 1.0f), 10, 10, 0.5f);

	bLightEnabled = false;
}

void SceneBase::Update(double dt)
{
	//Keyboard Section
	if(Application::IsKeyPressed('1'))
		glEnable(GL_CULL_FACE);
	if(Application::IsKeyPressed('2'))
		glDisable(GL_CULL_FACE);
	if(Application::IsKeyPressed('3'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(Application::IsKeyPressed('4'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	fps = (float)(1.f / dt);
}

void SceneBase::RenderText(Mesh* mesh, std::string text, Color color)
{
	if(!mesh || mesh->textureID <= 0)
		return;
	
	glDisable(GL_DEPTH_TEST);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	float accum = 0;
	for(unsigned i = 0; i < text.length(); ++i)
	{
		Mtx44 characterSpacing;
		characterSpacing.SetToTranslation(accum, 0, 0); //1.0f is the spacing of each character, you may change this value
		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);
	
		mesh->Render((unsigned)text[i] * 6, 6);
		accum += (float)fontWidth[(unsigned)text[i]] / 64;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	glEnable(GL_DEPTH_TEST);
}

void SceneBase::RenderTextOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y)
{
	if(!mesh || mesh->textureID <= 0)
		return;

	glDisable(GL_DEPTH_TEST);
	Mtx44 ortho;
	ortho.SetToOrtho(0, 80, 0, 60, -10, 10);
	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);
	viewStack.PushMatrix();
	viewStack.LoadIdentity();
	modelStack.PushMatrix();
	modelStack.LoadIdentity();
	modelStack.Translate(x, y, 0);
	modelStack.Scale(size, size, size);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	float accum = 0;
	for(unsigned i = 0; i < text.length(); ++i)
	{
		Mtx44 characterSpacing;
		characterSpacing.SetToTranslation(accum + 0.5f, 0.5f, 0); //1.0f is the spacing of each character, you may change this value
		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);

		mesh->Render((unsigned)text[i] * 6, 6);

		accum += (float)fontWidth[(unsigned)text[i]] / 64;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	modelStack.PopMatrix();
	viewStack.PopMatrix();
	projectionStack.PopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void SceneBase::RenderMesh(Mesh *mesh, bool enableLight)
{
	Mtx44 MVP, modelView, modelView_inverse_transpose;
	
	MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);
	if(enableLight && bLightEnabled)
	{
		glUniform1i(m_parameters[U_LIGHTENABLED], 1);
		modelView = viewStack.Top() * modelStack.Top();
		glUniformMatrix4fv(m_parameters[U_MODELVIEW], 1, GL_FALSE, &modelView.a[0]);
		modelView_inverse_transpose = modelView.GetInverse().GetTranspose();
		glUniformMatrix4fv(m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE], 1, GL_FALSE, &modelView.a[0]);
		
		//load material
		glUniform3fv(m_parameters[U_MATERIAL_AMBIENT], 1, &mesh->material.kAmbient.r);
		glUniform3fv(m_parameters[U_MATERIAL_DIFFUSE], 1, &mesh->material.kDiffuse.r);
		glUniform3fv(m_parameters[U_MATERIAL_SPECULAR], 1, &mesh->material.kSpecular.r);
		glUniform1f(m_parameters[U_MATERIAL_SHININESS], mesh->material.kShininess);
	}
	else
	{	
		glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	}
	if(mesh->textureID > 0)
	{
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh->textureID);
		glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	}
	else
	{
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 0);
	}
	mesh->Render();
	if(mesh->textureID > 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void SceneBase::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SceneBase::Exit()
{
	// Cleanup VBO
	for(int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if(meshList[i])
			delete meshList[i];
	}
	glDeleteProgram(m_programID);
	glDeleteVertexArrays(1, &m_vertexArrayID);
}
