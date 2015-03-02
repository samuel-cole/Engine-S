#ifndef TUTORIAL_5
#define TUTORIAL_5

#include "Application.h"
#include "glm\glm.hpp"

class FlyCamera;
class Renderer;

class Tutorial5 : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	FlyCamera* m_camera;
	Renderer* m_renderer;

};

#endif