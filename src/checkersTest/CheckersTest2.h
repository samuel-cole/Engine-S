#ifndef CHECKERS_TEST_2_H
#define CHECKERS_TEST_2_H

#include "physX\PhysicsBase.h"

#include <thread>

class PlayerCollisions;
class CheckersMover;

class CheckersTest2 : public PhysicsBase
{
public:
	inline float const GetAmplitude()
	{
		return m_amplitude;
	}
	inline unsigned int const GetSeed()
	{
		return m_seed;
	}
	inline float const GetPersistence()
	{
		return m_persistence;
	}
	inline Renderer* const GetRenderer()
	{
		return m_renderer;
	}
	inline unsigned int const GetProceduralPlane()
	{
		return m_proceduralPlane;
	}
	inline PxRigidStatic* const GetProceduralPhysics()
	{
		return g_proceduralPhysics;
	}
	inline PxMaterial* const GetPhysicsMaterial()
	{
		return g_physicsMaterial;
	}
	inline PxController* const GetPlayer()
	{
		return g_playerController;
	}
	inline void SetProceduralPlane(unsigned int a_object)
	{
		m_proceduralPlane = a_object;
	}
	inline void SetProceduralPhysics(PxRigidStatic* a_proceduralPhysics)
	{
		g_proceduralPhysics = a_proceduralPhysics;
	}
	//See PhysicsBase.h for an explanation of these arguments.
	inline PxRigidStatic* GenerateProceduralPlane(unsigned int a_dimensions, unsigned int a_noiseMapDimensions,
												  float a_stretch, const vec3& a_position, PxMaterial* a_material, unsigned int& a_rendererIndex, float& a_maxHeight,
												  float a_amplitude, unsigned int a_seed = rand(), unsigned int a_octaves = 6, float a_persistence = 0.3f)
	{
		return AddProceduralPlane(a_dimensions, a_noiseMapDimensions, a_stretch, a_position, a_material, a_rendererIndex, a_maxHeight, a_amplitude, a_seed, a_octaves, a_persistence);
	}

	void ResetCheckerPieceHeights();

private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	//Gets the height of the heightfield at the specified (x, z) coordinate. Returns a height value (or -1 if the coordinates are invalid).
	float GetHeightAtPos(float a_x, float a_z);

	PxMaterial* g_physicsMaterial;
	PxController* g_playerController;
	PlayerCollisions* g_playerCollisions;
	PxRigidStatic* g_proceduralPhysics;
	PxRigidStatic* g_plane;
	PxControllerManager* g_controllerManager;

	std::vector<PxRigidStatic*> g_teleporterBodies;

	float m_animationTimer;

	float m_spawnTimer;

	float m_walkSpeed;
	float m_verticleSpeed;
	bool m_canFly;
	
	float m_shootTimer;
	float m_shootForce;

	unsigned int m_gun;
	unsigned int m_animatedModel;
	unsigned int m_ground;

	std::vector<unsigned int> m_physicsLights;

	float m_amplitude;
	float m_persistence;
	unsigned int m_seed;
	unsigned int m_proceduralPlane;

	const float RAYCAST_HEIGHT = 400.0f;
	const PxVec3 RAYCAST_DIRECTION = PxVec3(0, -1, 0);

#pragma region Checkers Member Variables and Methods
	void CheckersUpdate(float a_deltaTime);

	void HandleEnter(int(&a_board)[8][8], const unsigned int a_xPos, const unsigned int a_yPos, unsigned int &a_prevX, unsigned int &a_prevY, bool &a_turn, const bool a_changeEmitters, unsigned int &a_pieceSelected);
	bool ValidMove(const int a_board[8][8], const unsigned int a_xPos, const unsigned int a_yPos, const unsigned int a_prevX, const unsigned int a_prevY, const bool a_turn, const bool a_changeEmitters);

	void UseAIMove();
	void AIMove(int(&a_board)[8][8], const bool a_turn, const unsigned int a_difficulty);
	//As vectors of arrays don't work, a vector<vector<int>>> has been used to replace a vector<int[8][8]>.
	//a_pieceToMove is used for outputting which piece has been moved in each possible move.
	std::vector<std::vector<std::vector<int>>> GetPossibleMoves(const int a_board[8][8], const bool a_turn, std::vector<std::tuple<unsigned int, unsigned int>>& a_pieceToMove = std::vector<std::tuple<unsigned int, unsigned int>>());
	//Plays random moves for both sides until the game ends.
	int PlayUntilEnd(std::vector<std::vector<int>> a_board, const bool a_turn);
	//Updates the visuals to match the board passed in as an argument- this re-updates the entire board, so when a known piece has been moved, it is better to update the visuals manually than call this function.
	void UpdateBoard();


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

	bool m_aiMoveFinished;
	bool m_threadFinished;

	bool m_turn;

	std::vector<CheckersMover*> m_movers;

	//These are used for telling the checkers mover which pieces to move.
	unsigned int m_xToMove;
	unsigned int m_yToMove;
#pragma endregion
};

#endif