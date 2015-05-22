#ifndef TUTORIAL_13
#define TUTORIAL_13

#include "Application.h"


void TW_CALL AddLight(void* a_clientData);

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

	unsigned int m_model;

	float m_timer;

};

#endif