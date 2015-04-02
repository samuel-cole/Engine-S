#ifndef TUTORIAL_12
#define TUTORIAL_12

#include "Application.h"

class FlyCamera;
class StaticCamera;
class Renderer;

#include <vector>

class Tutorial12 : public Application
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