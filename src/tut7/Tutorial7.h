#ifndef TUTORIAL_7
#define TUTORIAL_7

#include "Application.h"

class FlyCamera;
class Renderer;

class Tutorial7 : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	FlyCamera* m_camera;
	
	Renderer* m_renderer;

	unsigned int m_emitter;
	unsigned int m_emitter2;

	float m_timer;
};

#endif