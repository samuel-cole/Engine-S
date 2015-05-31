#ifndef CHECKERS_TEST_H
#define CHECKERS_TEST_H

#include "Application.h"

class FlyCamera;
class StaticCamera;
class Renderer;

#include <thread>
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

	void HandleEnter( int (&a_board)[8][8], const unsigned int a_xPos, const unsigned int a_yPos,	   unsigned int &a_prevX,	   unsigned int &a_prevY,	    bool &a_turn, const bool a_changeEmitters, unsigned int &a_pieceSelected);
	bool ValidMove(const int a_board[8][8], const unsigned int a_xPos, const unsigned int a_yPos, const unsigned int a_prevX, const unsigned int a_prevY,  const bool a_turn, const bool a_changeEmitters);

	void UseAIMove();
	void AIMove(int (&a_board)[8][8], const bool a_turn, const unsigned int a_difficulty);
	//As vectors of arrays don't work, a vector<vector<int>>> has been used to replace a vector<int[8][8]>
	std::vector<std::vector<std::vector<int>>> GetPossibleMoves(const int a_board[8][8], const bool a_turn);
	//Plays random moves for both sides until the game ends.
	int PlayUntilEnd(std::vector<std::vector<int>> a_board, const bool a_turn);

	FlyCamera* m_camera;

	InfoForBar m_infoForBar;

	std::vector<unsigned int> m_emitters;

	const float M_TILE_WIDTH = 12.5f;

	//The int at each location on the board represents which emitter is at that location.
	int m_board[8][8];
			
	unsigned int m_currentX;
	unsigned int m_currentY;

	unsigned int m_previousX;
	unsigned int m_previousY;

	unsigned int m_positionMarker;

	float m_inputTimer;

	unsigned int m_pieceSelected;

	std::thread m_aiThread;

	bool m_threadFinished;

	bool m_turn;
};

#endif