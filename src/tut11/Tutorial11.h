#ifndef TUTORIAL_11
#define TUTORIAL_11

#include "Application.h"

class FlyCamera;
class StaticCamera;
class Renderer;

#include <vector>

class Tutorial11 : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	float m_timer;

	FlyCamera* m_camera;
	std::vector<StaticCamera*> m_staticCamera;
	Renderer* m_renderer;
};

#endif