#ifndef TUTORIAL_13
#define TUTORIAL_13

#include "Application.h"


class FlyCamera;
class Renderer;

class Tutorial13 : public Application
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