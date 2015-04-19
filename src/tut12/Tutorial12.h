#ifndef TUTORIAL_12
#define TUTORIAL_12

#include "Application.h"

class FlyCamera;
class StaticCamera;
class Renderer;

#include <vector>

struct ButtonInfo {
	Renderer* renderer;
	float amplitude;
	float persistence;
	unsigned int seed;
	unsigned int object;
};

class Tutorial12 : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	FlyCamera* m_camera;

	ButtonInfo m_buttonInfo;

};

#endif