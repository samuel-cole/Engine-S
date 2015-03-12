#ifndef TUTORIAL_9
#define TUTORIAL_9

#include "Application.h"



class ParticleEmitter;
class FlyCamera;
class Renderer;

class Tutorial9 : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	float m_timer;

	unsigned int m_frameBuffer;

	FlyCamera* m_camera;
	Renderer* m_renderer;
	unsigned int m_emitter;

};

#endif