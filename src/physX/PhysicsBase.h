#ifndef PHYSICS_BASE
#define PHYSICS_BASE

#include "Application.h"

class WalkCamera;
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
	void AddBox(PxMaterial* const a_material, const float a_density, const vec3& a_dimensions, const vec3& a_position, const bool a_visible);
	void AddSphere(PxMaterial* const a_material, const float a_density, const float a_radius, const vec3& a_position, const bool a_visible);

	//Function used for adding a procedurally generated plane.
	//a_dimensions determines how many rows/columns the created mesh should have (also sets the size).
	//a_noiseMapDimensions determines how many rows/columns the perlin noise mesh should have. Set to the same value as a_dimensions for most purposes, use a smaller value for a 'blocky' look.
	//a_stretch determines how much the mesh should be scaled by.
	//a_position determines the position of the center of the plane.
	//a_material sets the physics material for the plane.
	//a_rendererIndex is used to output the index of this object within the renderer. Note that if this object is transformed in any way in the renderer, the physics object will no longer be valid.
	//a_maxHeight is used to output the highest height that the procedural plane reaches.
	//a_amplitude determines how high the plane should be deformed.
	//a_seed is used to seed the random generation. Different values result in a different randomly generated plane.
	//a_octaves determines how bumpy the plane should be- higher numbers are bumpier, but take longer to generate.
	//a_persistence determines how quickly the height deformation should fall-off during generation.
	//Returns the physics object of the plane.
	PxRigidStatic* AddProceduralPlane(const unsigned int a_dimensions, const unsigned int a_noiseMapDimensions,
									  const float a_stretch, const vec3& a_position, PxMaterial* const a_material, unsigned int& a_rendererIndex, float& a_maxHeight,
									  float a_amplitude, unsigned int a_seed = rand(), unsigned int a_octaves = 6, float a_persistence = 0.3f);

	//Function used for adding a piece of cloth.
	//a_dimensions determines how many rows/columns the created cloth should have (also sets the size).
	//a_staticPoints determines which points in the cloth should be static- points are determined according to the equation point = row * dimension + column. For example, to have a piece of 10x10 cloth that hangs from its top two corners, set this vector to contain 0 and 90.
	//a_pose determines the transform of the cloth.
	void AddCloth(unsigned int a_dimensions, std::vector<unsigned int> a_staticPoints, PxTransform a_pose);


	//Physics Variables
	PxFoundation* g_physicsFoundation;
	PxPhysics* g_physics;
	PxScene* g_physicsScene;
	PxDefaultErrorCallback g_defaultErrorCallback;
	PxAllocatorCallback* g_allocatorCallback;
	PxSimulationFilterShader g_defaultFilterShader = PxDefaultSimulationFilterShader;
	PxVisualDebuggerConnection* g_pvdConnection;
	PxCpuDispatcher* g_cpuDispatcher;

	//Terrain object. Stored for cleaning up purposes.
	PxRigidStatic* g_terrain;

	std::vector<PxRigidActor*> g_physicsActors;
	std::vector<PxCloth*> g_physicsCloths;

	//TEST
	PxCloth* m_cloth;
	unsigned int m_clothGrid;

	//End of physics variables

	//The models of each of the physics actors.
	std::vector<unsigned int> m_models;
	//The scale that each model should be at.
	std::vector<vec3> m_scales;

	std::vector<unsigned int> m_clothModels;

	WalkCamera* m_camera;
	Renderer* m_renderer;
};

#endif