#ifndef CHECKERS_TEST_2_H
#define CHECKERS_TEST_2_H

#include "physX\PhysicsBase.h"

#include <thread>

struct InfoForBar2 {
	Renderer* renderer;
	float amplitude;
	float persistence;
	unsigned int seed;
	unsigned int object;
	PxRigidStatic* physicsObject;
};

class CheckersTest2 : public PhysicsBase
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	PxMaterial* g_physicsMaterial;

	float m_spawnTimer;

	float m_shootTimer;
	float m_shootForce;

	unsigned int m_gun;

	std::vector<unsigned int> m_physicsLights;

	PxRigidDynamic* m_player;

	InfoForBar2 m_infoForBar;


#pragma region Checkers Member Variables and Methods
	void CheckersUpdate(float a_deltaTime);

	void HandleEnter(int(&a_board)[8][8], const unsigned int a_xPos, const unsigned int a_yPos, unsigned int &a_prevX, unsigned int &a_prevY, bool &a_turn, const bool a_changeEmitters, unsigned int &a_pieceSelected);
	bool ValidMove(const int a_board[8][8], const unsigned int a_xPos, const unsigned int a_yPos, const unsigned int a_prevX, const unsigned int a_prevY, const bool a_turn, const bool a_changeEmitters);

	void UseAIMove();
	void AIMove(int(&a_board)[8][8], const bool a_turn, const unsigned int a_difficulty);
	//As vectors of arrays don't work, a vector<vector<int>>> has been used to replace a vector<int[8][8]>
	std::vector<std::vector<std::vector<int>>> GetPossibleMoves(const int a_board[8][8], const bool a_turn);
	//Plays random moves for both sides until the game ends.
	int PlayUntilEnd(std::vector<std::vector<int>> a_board, const bool a_turn);


	std::vector<unsigned int> m_emitters;
	std::vector<unsigned int> m_pieceLights;

	const float M_TILE_WIDTH = 12.5f;

	//The int at each location on the board represents which emitter is at that location.
	int m_board[8][8];

	unsigned int m_currentX;
	unsigned int m_currentY;

	unsigned int m_previousX;
	unsigned int m_previousY;

	unsigned int m_positionMarker;
	unsigned int m_positionLight;
	unsigned int m_positionLight2;

	float m_colourTimer;

	float m_inputTimer;

	unsigned int m_pieceSelected;

	unsigned int m_aiDifficulty;

	std::thread m_aiThread;

	bool m_threadFinished;

	bool m_turn;
#pragma endregion
	

	//Variables for generating the random board.
	//float m_amplitude;
	//float m_persistence;
	//unsigned int m_seed;
};

#endif