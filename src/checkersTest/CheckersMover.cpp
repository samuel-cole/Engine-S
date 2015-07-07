#include "CheckersMover.h"

bool CheckersMover::s_checkersMoveAvailable = false;

CheckersMover::CheckersMover(float a_speed, float a_distanceModifier, float a_checkersModifier, float a_physicsModifier, float a_maxDistanceSquared, PxRigidDynamic* a_physicsBody) :
m_speed(a_speed), m_distanceModifier(a_distanceModifier), m_checkersModifier(a_checkersModifier), m_physicsModifier(a_physicsModifier), m_maxDistanceSquared(a_maxDistanceSquared), g_physicsBody(a_physicsBody)
{
}

void CheckersMover::Update(const PxVec3& a_physicsPos, const PxVec3& a_checkersPos)
{
	//The two actions available are 'mess around with the nearest physics object' and 'make a checkers move'. These variables are the utility scores of each.
	float physicsUtilityScore, checkersUtilityScore;

	PxVec3 myPos = g_physicsBody->getGlobalPose().p;

	//Geta value between 0 and 1 based on the distance of the closest physics object 
	float physicsDistanceValue = fmaxf(1 - (a_physicsPos - myPos).magnitudeSquared() / m_maxDistanceSquared, 0);
	//The '1.0f' part of this equation is there because there are always physics objects within the world.
	physicsUtilityScore = (m_physicsModifier * 1.0f) * (m_distanceModifier * physicsDistanceValue);

	if (s_checkersMoveAvailable)
	{
		float checkersDistanceValue = fmaxf(1 - (a_checkersPos - myPos).magnitudeSquared() / m_maxDistanceSquared, 0);
		//'1.0f' is used here because it is already known (because of the 'if' statement above) that there is a checkers move available. If there was not one, this would be 0.
		checkersUtilityScore = (m_checkersModifier * 1.0f) * (m_distanceModifier * checkersDistanceValue);
	}
	else
	{
		//The result will always be zero if there are no moves available.
		checkersUtilityScore = 0.0f;
	}
	
	//Apply a force based on which utility score is higher.
	g_physicsBody->addForce(m_speed * (myPos - (checkersUtilityScore > physicsUtilityScore ? a_checkersPos : a_physicsPos)).getNormalized());
	
}