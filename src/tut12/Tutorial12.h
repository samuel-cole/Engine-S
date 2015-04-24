#ifndef TUTORIAL_12_H
#define TUTORIAL_12_H

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
	unsigned int rows;
	unsigned int columns;
	unsigned int perlinRows;
	unsigned int perlinColumns;
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
	ButtonInfo m_buttonInfo2;

};

#endif