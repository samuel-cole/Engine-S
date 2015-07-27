#ifndef CHECKERS_H
#define CHECKERS_H

#include "PhysicsBase.h"

#include <thread>

class PlayerCollisions;
class CheckersMover;

//Checkers example class. Contains a scene that involves a checkers game, physics objects and many particles and lights.
class Checkers : public PhysicsBase
{
public:
	//Accessor functions. These are used to allow the debug bar to re-generate the procedural plane.
	//Returns the amplitude of the perlin noise used in the scene.
	inline float const GetAmplitude()
	{
		return m_amplitude;
	}
	//Returns the seed of the perlin noise in the scene.
	inline unsigned int const GetSeed()
	{
		return m_seed;
	}
	//Returns the persistence of the perlin noise in the scene.
	inline float const GetPersistence()
	{
		return m_persistence;
	}
	//Returns a pointer to the renderer used by the scene.
	inline Renderer* const GetRenderer()
	{
		return m_renderer;
	}
	//Returns a handle (for the renderer) to the procedural plane within the scene.
	inline unsigned int const GetProceduralPlane()
	{
		return m_proceduralPlane;
	}
	//Returns a pointer to the physics object used to represent the procedural plane within the scene.
	inline PxRigidStatic* const GetProceduralPhysics()
	{
		return g_proceduralPhysics;
	}
	//Returns a pointer to the physics material used within the scene.
	inline PxMaterial* const GetPhysicsMaterial()
	{
		return g_physicsMaterial;
	}
	//Returns a pointer to the physics controller used for the player.
	inline PxController* const GetPlayer()
	{
		return g_playerController;
	}
	
	//Mutator Functions. These are used to allow the debug bar to re-generate the procedural plane.
	//Sets the renderer handle to the procedural plane to the handle passed as an argument.
	inline void SetProceduralPlane(unsigned int a_object)
	{
		m_proceduralPlane = a_object;
	}
	//Sets the physics object used for the procedural plane to the object passed in as an argument.
	inline void SetProceduralPhysics(PxRigidStatic* a_proceduralPhysics)
	{
		g_proceduralPhysics = a_proceduralPhysics;
	}
	//Generates a procedural plane within the scene- wraps the 'AddProceduralPlane' function within PhysicsBase.
	//See 'AddProceduralPlane()' within PhysicsBase.h for an explanation of these arguments.
	inline PxRigidStatic* GenerateProceduralPlane(unsigned int a_dimensions, unsigned int a_noiseMapDimensions,
												  float a_stretch, const vec3& a_position, PxMaterial* a_material, unsigned int& a_rendererIndex, float& a_maxHeight,
												  float a_amplitude, unsigned int a_seed = rand(), unsigned int a_octaves = 6, float a_persistence = 0.3f)
	{
		return AddProceduralPlane(a_dimensions, a_noiseMapDimensions, a_stretch, a_position, a_material, a_rendererIndex, a_maxHeight, a_amplitude, a_seed, a_octaves, a_persistence);
	}

	//Sets the heights of the checker pieces to values slightly above the procedural plane- should be called after modifying the procedural plane.
	void ResetCheckerPieceHeights();

private:
	//Called when the game starts. Sets up the checkers board, the player, and various physics objects.
	virtual int Init();
	//Called when the game ends. Handles cleaning up everything.
	virtual int Deinit();
	//Called every frame before Draw(). Handles all game logic (checkers, physics objects spawning, player movement/shooting, etc.)
	virtual void Update(float a_deltaTime);
	//Called every frame after Update()- handles rendering the scene.
	virtual void Draw();

	//Gets the height of the heightfield at the specified (x, z) coordinate. Returns a height value (or -1 if the coordinates are invalid).
	float GetHeightAtPos(float a_x, float a_z);

	//The material used for all physics objects within the scene.
	PxMaterial* g_physicsMaterial;
	//The controller used for the player.
	PxController* g_playerController;
	//Hit report that controls how collisions with the player are handled.
	PlayerCollisions* g_playerCollisions;
	//The physics object for the procedurally generated checkerboard in the scene.
	PxRigidStatic* g_proceduralPhysics;
	//The physics object used for the ground.
	PxRigidStatic* g_plane;
	//The controller manager used for managing the player controller.
	PxControllerManager* g_controllerManager;

	//The physics bodies for all of the teleporters within the scene.
	std::vector<PxRigidStatic*> g_teleporterBodies;

	//Timer that tracks what time the animated model should be at in its animation.
	float m_animationTimer;

	//The time until the next physics object is spawned.
	float m_spawnTimer;

	//The speed the player moves at.
	float m_walkSpeed;
	//The current verticle speed the player should move at- this is used because the player controller doesn't account for gravity.
	float m_verticleSpeed;
	//Option that removes the limit on jumping in mid air, and reduces the gravity the player experiences.
	bool m_canFly;
	
	//The amount of time until the player can shoot again.
	float m_shootTimer;
	//The amount of force that shots leave the player's gun/crossbow.
	float m_shootForce;

	//Renderer handle to the model for the gun/crossbow the player has.
	unsigned int m_gun;
	//Renderer handle to the animated model within the scene.
	unsigned int m_animatedModel;
	//Renderer handle to the plane used for the ground.
	unsigned int m_ground;

	//Vector of renderer handles to the lights attached to each physics object.
	std::vector<unsigned int> m_physicsLights;

	//The amplitude setting of the perlin-generated procedural plane- determines the height of the peaks. 0-100 is a good range of values.
	float m_amplitude;
	//The persistence setting of the perlin-generated procedural plane- determines how bumpy the plane is, and also the height of the peaks. 0.1-0.5 is a good range of values.
	float m_persistence;
	//The seed used for the perlin-generated procedural plane. Any value is good.
	unsigned int m_seed;
	//Renderer handle to the perlin-generated procedural plane.
	unsigned int m_proceduralPlane;

	//The height that raycasts used for piece height-checking should start at.
	const float RAYCAST_HEIGHT = 400.0f;
	//The direction that piece height-checking rays should face in.
	const PxVec3 RAYCAST_DIRECTION = PxVec3(0, -1, 0);

#pragma region Checkers Member Variables and Methods
	//Called every frame, CheckersUpdate manages checkers logic and player input each frame.
	void CheckersUpdate(float a_deltaTime);

	//Handles what should happen when the player presses the 'enter' key. Takes the following arguments (anything not const is an output as well as an input):
	//a_board: The current board state.
	//a_xPos, a_yPos: The location on the board of the player's selector token.
	//a_prevX, a_prevY: The location of the currently selected piece- used as input if a piece is selected before calling this function, or output if one is not.
	//a_turn: The player whose turn it currently is. False for blue player, true for red.
	//a_changeEmitters: Whether to modify the main, visible board after making a move.
	//a_pieceSelected: The piece currently selected.
	void HandleEnter(int(&a_board)[8][8], const unsigned int a_xPos, const unsigned int a_yPos, unsigned int &a_prevX, unsigned int &a_prevY, bool &a_turn, const bool a_changeEmitters, unsigned int &a_pieceSelected);

	//Checks whether the current move is a valid one or not. Returns whether the move is valid. Takes the following arguments:
	//a_board: The current board state.
	//a_xPos, a_yPos: The location on the board of the player's selector token.
	//a_prevX, a_prevY: The location of the currently selected piece.
	//a_turn: The player whose turn it currently is. False for blue, true for red.
	//a_changeEmitters: Whether or not this function should modify the board. The only modification that this function will make if set to true is that if the move is a valid capturing move, the captured piece will be removed.
	bool ValidMove(const int a_board[8][8], const unsigned int a_xPos, const unsigned int a_yPos, const unsigned int a_prevX, const unsigned int a_prevY, const bool a_turn, const bool a_changeEmitters);

	//Handles the AI making a checkers move- calls AIMove(), makes sure that the AI is only used once per turn, and informs the checkers movers when the AI has made its move.
	void UseAIMove();
	//Makes a move for the AI player- takes in a board, which turn it is, and what difficulty the AI should play at (10-100 is a good range for the difficulty to be within).
	void AIMove(int(&a_board)[8][8], const bool a_turn, const unsigned int a_difficulty);
	//Gets all of the possible moves that a player could make. Used for AI calculations.
	//Returns a vector of board states- as vectors of arrays don't work, a vector<vector<vector<int>>>> has been used to replace a vector<int[8][8]>.
	//a_pieceToMove is used for outputting which piece has been moved in each possible move.
	std::vector<std::vector<std::vector<int>>> GetPossibleMoves(const int a_board[8][8], const bool a_turn, std::vector<std::tuple<unsigned int, unsigned int>>& a_pieceToMove = std::vector<std::tuple<unsigned int, unsigned int>>());
	//Plays random moves for both sides until the game ends.
	int PlayUntilEnd(std::vector<std::vector<int>> a_board, const bool a_turn);
	//Updates the visuals to match m_board- this re-updates the entire board, so when a known piece has been moved, it is better to update the visuals manually than call this function.
	void UpdateBoard();

	//Vector containing renderer handles to the emitters used for the checkers pieces.
	std::vector<unsigned int> m_emitters;
	//Vector containing renderer handles to the lights placed at each checkers piece's location.
	std::vector<unsigned int> m_pieceLights;

	//The width of one tile in the renderer's units of distance- used for placing the emitters/lights/etc. at their correct locations.
	const float M_TILE_WIDTH = 12.5f;

	//The current board state- the int at each location on the board represents which emitter is at that location, and -1 is used for empty spaces.
	int m_board[8][8];

	//The current location (measured in board tiles) of the player's selector token.
	unsigned int m_currentX;
	unsigned int m_currentY;

	//The location (measured in board tiles) of the currently selected piece.
	unsigned int m_previousX;
	unsigned int m_previousY;

	//Renderer handle to the object used for the player's selector token,
	unsigned int m_positionMarker;
	//Renderer handles to the lights next to the player's selector token.
	unsigned int m_positionLight;
	unsigned int m_positionLight2;

	//The amount of time until the lights connected to the player's selector token should switch colour.
	float m_colourTimer;

	//The amount of time until the game should accept input again- used as a quick way to prevent the player's inputs from taking effect multiple times.
	float m_inputTimer;

	//Renderer handle to the currently selected piece.
	unsigned int m_pieceSelected;

	//The current difficulty level of the AI. Higher numbers are harder. 10-100 is a good range of values.
	unsigned int m_aiDifficulty;

	//The thread used for running the AI in.
	std::thread m_aiThread;

	//Whether the AI has finished making its move- is set to false when the move starts, then set to true after the checkers movers have finished making the move.
	bool m_aiMoveFinished;
	//Whether the AI thread has finished deciding which move to make. Is set to false when the move starts, then set to true after the ai has finished deciding a move (before the checkers movers actually make the move).
	bool m_threadFinished;

	//The player whose turn it currently is. False for blue player, true for red.
	bool m_turn;

	//Vector containing all of the checkers movers within the scene- ai entities that move checkers pieces and interact which physics within the scene.
	std::vector<CheckersMover*> m_movers;

	//These are used for telling the checkers mover which pieces to move.
	unsigned int m_xToMove;
	unsigned int m_yToMove;
#pragma endregion
};

#endif