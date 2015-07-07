#ifndef CHECKERS_MOVER_H
#define CHECKERS_MOVER_H

#include <PxPhysicsAPI.h>

using namespace physx;

//This class is used for controlling AI agents that will move the checkers pieces around. While not moving checkers pieces, they will push physics objects around.
class CheckersMover
{
private:
	PxRigidDynamic* g_physicsBody;

	//Utility modifiers
	float m_distanceModifier, m_checkersModifier, m_physicsModifier;
	//The cut-off distance at which objects are no longer considered.
	float m_maxDistanceSquared;

	float m_speed;

	static bool s_checkersMoveAvailable;
public:
	//Constructor for creating a checker mover. 
	//Modifier values impact how important that element is in this agent's decisions, and should range from 0 to 1.
	//'a_maxDistanceSquared' is used for determining the maximum range that this mover will detect actions within.
	//'a_physicsBody' is the physics body for this checker mover.
	CheckersMover(float a_speed, float a_distanceModifier, float a_checkersModifier, float a_physicsModifier, float a_maxDistanceSquared, PxRigidDynamic* a_physicsBody);
	//Update function; should be called every frame. Pass in the position of the closest physics object, and the position of the checkers piece that needs moving (if there is one), as arguments.
	void Update(const PxVec3& a_physicsPos, const PxVec3& a_checkersPos = PxVec3(0, 0, 0));

	//Call this when a move has been made in checkers.
	inline static void MoveAvailable()
	{
		s_checkersMoveAvailable = true;
	}
};

#endif