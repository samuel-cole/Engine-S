#ifndef APPLICATION_H
#define APPLICATION_H

#include "AntTweakBar.h"

struct GLFWwindow;

class Application
{
public:
	virtual int Run();
protected:
	virtual int Init();
	virtual int Deinit();
	virtual void Update(float a_deltaTime) = 0;
	virtual void Draw() = 0;

	GLFWwindow* m_window;

	TwBar* m_debugBar;

	float m_currentTime;
	float m_deltaTime;
	float m_previousTime;


};

#endif