#ifndef LEVEL_DATA_H
#define LEVEL_DATA_H

#include <vector>

class PhysicsGame;

//=======================================================
//FleX project addition
//=======================================================
//This class sets each of the properties for each level in the game.
//When loading a level, there are three things to set- 
//the objects in the scene, the modifiable properties, and the indices for the goal object and target shape.
class LevelData
{
public:
	//Loads the specified level, returns the modifiable properties mask for the level created.
	//a_goalObject and a_targetShape are out parameters for the indexes of the goal object and the target shape.
	//Set goal object to -2 to treat all fluids in the scene as goal objects.
	static char LoadLevel(PhysicsGame* a_game, int a_level, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes);

private:
	//Loads the data for level 0- a sandbox level, featuring most elements of the game in a simple puzzle.
	static char Level0(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes);
	//Loads the data for level 1- a puzzle to teach the player usage of the pause/unpause button, in which the player just needs to unpause the game.
	static char Level1(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes);
	//Loads the data for level 2- the death-box level, in which the player starts surrounded by hazard objects on all bar one side, 
	//and must stop time at the start of the game in order to re-orient gravity to escape the box.
	static char Level2(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes);
	//Loads the data for level 3- a puzzle in which the goal is hidden on the ceiling, which introduces fluid as a goal object.
	static char Level3(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes);


	LevelData() {};
};

#endif