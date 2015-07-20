#ifndef PHYSICS_CONCRETE_2
#define PHYSICS_CONCRETE_2

#include "PhysicsBase.h"

class PhysicsConcrete2 : public PhysicsBase
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	PxMaterial* g_physicsMaterial;

	float m_spawnTimer;

	float m_shootTimer;
	float m_shootForce;

	unsigned int m_gun;

	float m_walkSpeed;
	float m_verticleSpeed;
	bool m_canFly;

	PxController* g_playerController;

	PxCloth* m_cloth;
	unsigned int m_clothGrid;

	std::vector<unsigned int> m_lights;

	PxRigidDynamic* m_player;
};


#endif