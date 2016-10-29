#ifndef PHYSICS_GAME_H
#define PHYSICS_GAME_H

#include "FleXBase.h"

class PhysicsGame : public FleXBase
{
public:
	enum PropertyTypes
	{
		GRAVITY,
		RESTITUTION,
		FRICTION,
	};

private:
	int Init() override;

	void Update(float a_deltaTime) override;

	void CheckWin();

	//Loads the level specified as an argument. Starting game argument is for whether this is the first level loaded in, or if another level already exists.
	void LoadLevel(const int a_level, const bool a_startingGame = false);

	int m_loadedLevel;

	//Index of the goal object within the vectors of objects in the scene.
	unsigned int m_goalObjectIndex;
	//Index of the target area shape within FleX.
	unsigned int m_targetShapeIndex;
	//Indices of each hazard object within the scene.
	std::vector<unsigned int> m_hazardShapeIndices;

	//Render handle to the point light used with the goal object.
	unsigned int m_goalObjectLightIndex;

	FlexParams g_params;

	vec3 m_gravityDir;
	float m_gravityStrength;

	vec3 m_oldGravityDir;
	float m_oldGravityStrength;

	float m_restitution;
	float m_oldRestitution;

	float m_currentLevelTime;

	char m_modifiablePropertiesMask;
};

#endif