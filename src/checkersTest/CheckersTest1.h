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
};

class CheckersTest : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	void HandleEnter();

	FlyCamera* m_camera;

	InfoForBar m_infoForBar;

	std::vector<unsigned int> m_emitters;

	const float M_TILE_WIDTH = 12.5f;

	//The unsigned int at each location on the board represents which emitter is at that location.
	unsigned int m_board[8][8];
			
	unsigned int m_currentX;
	unsigned int m_currentY;

	unsigned int m_positionMarker;

	float m_inputTimer;

	unsigned int m_pieceSelected;
};

#endif