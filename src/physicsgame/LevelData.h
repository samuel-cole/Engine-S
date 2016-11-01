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
	static char LoadLevel(PhysicsGame* a_game, int a_level, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes);

private:
	//Loads the data for level 0- a test level used debugging any changes to the game or handling of FleX.
	static char Level0(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes);
	//Loads the data for level 1- a puzzle in which the player must stop time, then set gravity to up in order to avoid some hazard objects. 
	static char Level1(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes);

	LevelData() {};
};

#endif