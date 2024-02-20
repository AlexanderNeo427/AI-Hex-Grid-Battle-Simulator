
#ifndef APPLICATION_H
#define APPLICATION_H

#include "timer.h"
#include "SceneA2.h"

class Application
{
public:
	static Application& GetInstance()
	{
		static Application app;
		return app;
	}
	void Init();
	void Run();
	void Iterate();
	void Exit();
	static bool IsKeyPressed(unsigned short key);
	static bool IsMousePressed(unsigned short key);
	static void GetCursorPos(double *xpos, double *ypos);
	static int GetWindowWidth();
	static int GetWindowHeight();
private:
	Application();
	~Application();

	// Declare a window object
	StopWatch m_timer;

	SceneA2* scene;
};

#endif