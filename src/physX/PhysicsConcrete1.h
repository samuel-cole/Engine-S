#ifndef PHYSICS_CONCRETE_1
#define PHYSICS_CONCRETE_1

#include "PhysicsBase.h"

class PhysicsConcrete1 : public PhysicsBase
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	PxMaterial* g_physicsMaterial;

	float m_spawnTimer;
	float m_shootTimer;

	unsigned int m_gun;

	std::vector<unsigned int> m_lights;

	PxRigidDynamic* m_player;
};


#endif