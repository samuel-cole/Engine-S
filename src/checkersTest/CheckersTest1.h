#ifndef CHECKERS_TEST_H
#define CHECKERS_TEST_H

#include "Application.h"

class FlyCamera;
class StaticCamera;
class Renderer;

#include <vector>

struct InfoForBar {
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

class CheckersTest : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	FlyCamera* m_camera;

	InfoForBar m_infoForBar;

	std::vector<unsigned int> m_emitters;

	float m_tileWidth;

	//Checkers ideas:
	//Framebuffers as one side's pieces.
	//More particles.
};

#endif