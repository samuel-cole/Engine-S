#ifndef TUTORIAL_2
#define TUTORIAL_2

#include "Application.h"
#include "glm\glm.hpp"

class FlyCamera;

class Tutorial2 : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	FlyCamera* m_camera;

};

#endif