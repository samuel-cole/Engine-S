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

protected:
	virtual int Init() override;

	virtual void Update(float a_deltaTime) override;

	void CheckWin();

	//Index of the goal object within the vectors of objects in the scene.
	unsigned int m_goalObjectIndex;
	//Index of the target area shape within FleX.
	unsigned int m_targetShapeIndex;
	//Indices of each hazard object within the scene.
	std::vector<unsigned int> m_hazardShapeIndices;

	FlexParams g_params;

	vec3 m_gravityDir;
	float m_gravityStrength;

	vec3 m_oldGravityDir;
	float m_oldGravityStrength;

	char m_modifiablePropertiesMask;
};

#endif