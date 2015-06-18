#ifndef PHYSICS_BASE
#define PHYSICS_BASE

#include "Application.h"

class FlyCamera;
class Renderer;

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxVisualDebugger.h>
#include <vector>
#include "glm/vec3.hpp"

using namespace physx;
using glm::vec3;

//Rigid Body Dynamics Tutorial.
class PhysicsBase : public Application
{
protected:
	virtual int Init();
	virtual int Deinit();
	virtual void Update(float a_deltaTime);
	virtual void Draw();

	void SetUpPhysX();
	void SetUpVisualDebugger();
	void UpdatePhysX(float a_deltaTime);
	void AddBox(PxMaterial* a_material, float a_density, vec3 a_dimensions, vec3 a_position, bool a_visible);
	void AddSphere(PxMaterial* a_material, float a_density, float a_radius, vec3 a_position, bool a_visible);

	//Physics Variables
	PxFoundation* g_physicsFoundation;
	PxPhysics* g_physics;
	PxScene* g_physicsScene;
	PxDefaultErrorCallback g_defaultErrorCallback;
	PxDefaultAllocator g_defaultAllocatorCallback;
	PxSimulationFilterShader g_defaultFilterShader = PxDefaultSimulationFilterShader;
	PxCooking* g_physicsCooker;

	std::vector<PxRigidActor*> g_physicsActors;
	//End of physics variables

	//The models of each of the physics actors.
	std::vector<unsigned int> m_models;
	//The scale that each model should be at.
	std::vector<vec3> m_scales;

	FlyCamera* m_camera;
	Renderer* m_renderer;
};

#endif