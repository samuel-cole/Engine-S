#ifndef TUTORIAL_8
#define TUTORIAL_8

#include "Application.h"

class FlyCamera;
class Renderer;

class Tutorial8 : public Application
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

	bool particleDirection;
};

#endif