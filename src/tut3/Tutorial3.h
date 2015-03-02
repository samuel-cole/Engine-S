#ifndef TUTORIAL_3
#define TUTORIAL_3

#include "Application.h"
#include "glm\glm.hpp"

class FlyCamera;
class Renderer;

class Tutorial3 : public Application
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