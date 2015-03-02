#ifndef TUTORIAL_4
#define TUTORIAL_4

#include "Application.h"


class FlyCamera;
class Renderer;

class Tutorial4 : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	FlyCamera* m_camera;
	Renderer* m_renderer;

	bool m_normalToggle;

};

#endif