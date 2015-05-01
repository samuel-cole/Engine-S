#include "CheckersTest1.h"
#include "glm\ext.hpp"
#include "StaticCamera.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "AntTweakBar.h"
#include "InputManager.h"
#include <iostream>

void TW_CALL BoardGenerate(void* a_clientData)
{
	InfoForBar* infoForBar = (InfoForBar*)a_clientData;

	infoForBar->renderer->DestroyObject(infoForBar->object);
	infoForBar->object = infoForBar->renderer->GenerateGrid(100, 100);
	infoForBar->renderer->LoadTexture("../data/checkerboard.png", infoForBar->object);
	infoForBar->renderer->GeneratePerlinNoiseMap(9, 9, 6, infoForBar->amplitude, infoForBar->persistence, infoForBar->object, infoForBar->seed, false);
}

int CheckersTest::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_infoForBar.renderer = new Renderer(m_camera, m_debugBar);

	m_infoForBar.amplitude = 3.0f;
	m_infoForBar.persistence = 0.3f;
	m_infoForBar.seed = 0;
	m_infoForBar.object = m_infoForBar.renderer->GenerateGrid(100, 100);
	m_infoForBar.renderer->LoadTexture("../data/checkerboard.png", m_infoForBar.object);
	m_infoForBar.renderer->GeneratePerlinNoiseMap(9, 9, 6, m_infoForBar.amplitude, m_infoForBar.persistence, m_infoForBar.object, m_infoForBar.seed, false);

	//M_TILE_WIDTH = 100.0f / 8.0f;
	
	TwAddSeparator(m_debugBar, "Plane 1", "");
	TwAddVarRW(m_debugBar, "Amplitude", TW_TYPE_FLOAT, &m_infoForBar.amplitude, "");
	TwAddVarRW(m_debugBar, "Persistence", TW_TYPE_FLOAT, &m_infoForBar.persistence, "");
	TwAddVarRW(m_debugBar, "Seed", TW_TYPE_UINT32, &m_infoForBar.seed, "");
	TwAddButton(m_debugBar, "Re-generate", BoardGenerate, (void*)&m_infoForBar, "");

	for (unsigned int i = 0; i < 24; ++i)
	{
		unsigned int emitter = m_infoForBar.renderer->CreateEmitter(1000, //Max particles
			0.1f,			//Lifespan minimum 
			1.0f,			//Lifespan maximum
			0.1f,			//Velocity minimum
			10.0f,			//Velocity maximum
			1.0f,			//Start size
			0.1f,			//End size
			((i < 12) ? vec4(1, 0, 0, 1) : vec4(0, 0, 1, 1)), //Start colour
			((i < 12) ? vec4(1, 1, 0, 1) : vec4(0, 1, 1, 1)), //End colour
			vec3(1, 0, 0),	//Direction
			3.14159265358979f,//Direction variance
			true,
			m_debugBar);			//GPU based

		m_emitters.push_back(emitter);
	}

	for (unsigned int i = 0; i < 8; ++i)
	{
		for (unsigned int j = 0; j < 8; ++j)
		{
			m_board[i][j] = -1;
		}
	}

	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i], true, vec3(M_TILE_WIDTH * -3.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * -2.5f));
		m_board[i * 2][1] = i;
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 12], true, vec3(M_TILE_WIDTH * -3.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * 1.5f));
		m_board[i * 2][5] = i + 12;
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 16], true, vec3(M_TILE_WIDTH * -3.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * 3.5f));
		m_board[i * 2][7] = i + 16;
	}

	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 4], true, vec3(M_TILE_WIDTH * -2.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * -3.5f));
		m_board[1 + i * 2][0] = i + 4;
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 8], true, vec3(M_TILE_WIDTH * -2.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * -1.5f));
		m_board[1 + i * 2][2] = i + 8;
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 20], true, vec3(M_TILE_WIDTH * -2.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * 2.5f));
		m_board[1 + i * 2][6] = i + 20;
	}

	m_currentX = 7;
	m_currentY = 7;

	m_positionMarker = m_infoForBar.renderer->LoadOBJ("../data/sphere/sphere.obj");
	m_infoForBar.renderer->LoadTexture("../data/crate.png", m_positionMarker);
	m_infoForBar.renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * 3.5f, 10, M_TILE_WIDTH * 3.5f)), m_positionMarker);

	m_inputTimer = 0;

	m_pieceSelected = -1;

	m_turn = false;

	return 0;
}

void CheckersTest::Update(float a_deltaTime)
{
	/*Stuff to add:
	Actual Game
	Pieces change height based on board
	Networked multiplayer
	Framebuffers as pieces
	Pieces lerp to position instead of just blink
	Proper input handling instead of a timer
	Line up taken pieces
	Special effects for multiple pieces taken in one turn.
	Provide options, eg: must take if you can on/off, etc.
	Show which pieces can be taken
	Don't select a piece at all it cannot make a move
	For kings, could just +number of pieces 
	*/

	m_inputTimer -= a_deltaTime;

	if (m_inputTimer < 0)
	{
		if (InputManager::GetKey(Keys::LEFT))
		{
			if (m_currentX > 0)
			{
				--m_currentX;
				m_inputTimer = 0.15f;
				m_infoForBar.renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY)), m_positionMarker);
			}
		}
		if (InputManager::GetKey(Keys::RIGHT))
		{
			if (m_currentX < 7)
			{
				++m_currentX;
				m_inputTimer = 0.15f;
				m_infoForBar.renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY)), m_positionMarker);
			}
		}
		if (InputManager::GetKey(Keys::UP))
		{
			if (m_currentY > 0)
			{
				--m_currentY;
				m_inputTimer = 0.15f;
				m_infoForBar.renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY)), m_positionMarker);
			}
		}
		if (InputManager::GetKey(Keys::DOWN))
		{
			if (m_currentY < 7)
			{
				++m_currentY;
				m_inputTimer = 0.15f;
				m_infoForBar.renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY)), m_positionMarker);
			}
		}
		if (InputManager::GetKey(Keys::ENTER))
		{
			HandleEnter();
			m_inputTimer = 0.15f;
		}
	}

	m_camera->Update(a_deltaTime);
}

void CheckersTest::Draw()
{
	m_infoForBar.renderer->Draw();
}

int CheckersTest::Deinit()
{
	delete m_camera;

	m_infoForBar.renderer->CleanupBuffers();

	return Application::Deinit();
}

void CheckersTest::HandleEnter()
{
	if (m_pieceSelected == -1)
	{
		if (m_board[m_currentX][m_currentY] != -1)
		{
			if ((m_turn && m_board[m_currentX][m_currentY] < 12) || (!m_turn && m_board[m_currentX][m_currentY] >= 12))
			{
				m_pieceSelected = m_board[m_currentX][m_currentY];
				m_infoForBar.renderer->SetEmitterPosition(m_emitters[m_pieceSelected], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
				m_previousX = m_currentX;
				m_previousY = m_currentY;
			}
			else
			{
				std::cout << "Invalid Move: Cannot Select Opponents Piece!" << std::endl;
			}
		}
	}
	else
	{
		if (ValidMove())
		{
			m_infoForBar.renderer->SetEmitterPosition(m_emitters[m_pieceSelected], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 5, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
			m_board[m_previousX][m_previousY] = -1;
			m_board[m_currentX][m_currentY] = m_pieceSelected;
			m_pieceSelected = -1;
			m_turn = !m_turn;
		}
		else
		{
			m_infoForBar.renderer->SetEmitterPosition(m_emitters[m_pieceSelected], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_previousX, 5, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_previousY));
			m_pieceSelected = -1;
		}
	}
}

bool CheckersTest::ValidMove()
{
	//I should later account for kings or whatever the upgraded checker pieces are called.

	//Remember to revise my code at some point to make sure that < 12 checks aren't breaking everything by selecting -1 tiles.

	//Capture move
	if ((m_currentX == m_previousX + 2 || m_currentX == m_previousX - 2) && ((m_currentY == m_previousY + 2 && m_pieceSelected < 12) || (m_currentY == m_previousY - 2 && m_pieceSelected >= 12)))
	{
		if (m_board[m_currentX][m_currentY] != -1)
		{
			std::cout << "Invalid Move: Square Occupied!" << std::endl;
			return false;
		}
		unsigned int takenSquare = m_board[(m_currentX + m_previousX) / 2][(m_currentY + m_previousY) / 2];
		if (takenSquare == -1)
		{
			std::cout << "Invalid Move: Jump Move Requires Piece To Jump!" << std::endl;
			return false;
		}
		if ((takenSquare < 12) == (m_board[m_previousX][m_previousY] < 12))
		{
			std::cout << "Invalid Move: Jumped Piece Must Belong To Opponent" << std::endl;
			return false;
		}
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[takenSquare], true, vec3(M_TILE_WIDTH * 4.5f * ((takenSquare < 12) ? -1 : 1), 5, 0));
		m_board[(m_currentX + m_previousX) / 2][(m_currentY + m_previousY) / 2] = -1;
		return true;
	}
	else
	{
		//First of all, do a check to see if a capture move is possible- if it is, then tell the player that their move is invalid, because 'take a piece' moves have already been checked for.
		for (unsigned int i = 0; i < 8; ++i)
		{
			for (unsigned int j = 0; j < 8; ++j)
			{
				//Check to see if player 1 can take player 2.
				if (m_turn)
				{
					if (m_board[i][j] != -1 && m_board[i][j] < 12)
					{
						//Check each direction for opponent pieces
						//Whoops- made it check backwards as well as forwards. Commented out lines should only apply when kings are involved.
						if ((i < 6 && j < 6 && m_board[i + 1][j + 1] >= 12 && m_board[i + 2][j + 2] == -1) ||
							/*(i < 6 && j > 1 && m_board[i + 1][j - 1] >= 12 && m_board[i + 2][j - 2] == -1) ||*/
							(i > 1 && j < 6 && m_board[i - 1][j + 1] >= 12 && m_board[i - 2][j + 2] == -1) /*||
																										   (i > 1 && j > 1 && m_board[i - 1][j - 1] >= 12 && m_board[i - 2][j - 2] == -1)*/)
						{
							std::cout << "Invalid Move: Must Capture Opponent Piece!" << std::endl;
							return false;
						}
					}
				}
				//Check to see if player 2 can take player 1
				else
				{
					if (m_board[i][j] >= 12)
					{
						//Check each direction for opponent pieces
						//Whoops- made it check backwards as well as forwards. Commented out lines should only apply when kings are involved.
						//There is an error in this one- not sure what yet though.
						if (//(i < 6 && j < 6 && m_board[i + 1][j + 1] < 12 && m_board[i + 1][j - 1] != -1 && m_board[i + 2][j + 2] == -1) ||
							(i < 6 && j > 1 && m_board[i + 1][j - 1] < 12 && m_board[i + 1][j - 1] != -1 && m_board[i + 2][j - 2] == -1) ||
							//(i > 1 && j < 6 && m_board[i - 1][j + 1] < 12 && m_board[i - 1][j - 1] != -1 && m_board[i - 2][j + 2] == -1) ||
							(i > 1 && j > 1 && m_board[i - 1][j - 1] < 12 && m_board[i - 1][j - 1] != -1 && m_board[i - 2][j - 2] == -1))
						{
							std::cout << "Invalid Move: Must Capture Opponent Piece!" << std::endl;
							return false;
						}
					}
				}
			}
		}

		//Default move
		if ((m_currentX == m_previousX + 1 || m_currentX == m_previousX - 1) && ((m_currentY == m_previousY + 1 && m_pieceSelected < 12) || (m_currentY == m_previousY - 1 && m_pieceSelected >= 12)))
		{
			if (m_board[m_currentX][m_currentY] != -1)
			{
				std::cout << "Invalid Move: Square Occupied!" << std::endl;
				return false;
			}
			return true;
		}
		std::cout << "Invalid Move!" << std::endl;
		return false;
	}

}