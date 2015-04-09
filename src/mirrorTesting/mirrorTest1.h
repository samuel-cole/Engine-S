#ifndef MIRROR_TEST_1
#define MIRROR_TEST_1

#include "Application.h"

class FlyCamera;
class StaticCamera;
class Renderer;

#include <vector>

class MirrorTest1 : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	float m_timer;

	FlyCamera* m_camera;
	StaticCamera* m_staticCamera;
	Renderer* m_renderer;

	unsigned int m_model;
	unsigned int m_mirror;
};

#endif