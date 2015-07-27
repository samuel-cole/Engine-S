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

//Base class for physics applications- handles a lot of physX set-up stuff, and provides functions for easy creation of basic physX types.
class PhysicsBase : public Application
{
protected:
	//Called when the game starts. Calls SetUpPhysX, and creates a renderer and camera.
	virtual int Init();
	//Called when the game ends. Cleans up PhysX and various variables.
	virtual int Deinit();
	//Called each frame, before Draw(). Updates the camera and calls UpdatePhysX().
	virtual void Update(float a_deltaTime);
	//Called each frame, after Update(). Draws the scene.
	virtual void Draw();

	//Sets up PhysX. Initialises all of the necessary objects.
	void SetUpPhysX();
	//Sets up the Visual Debugger. This allows the scene to be rendered in the PhysX Visual Debugger (seperate program).
	void SetUpVisualDebugger();
	//Updates PhysX, then updates all rendered objects so that they are shown where they are in the physics world.
	void UpdatePhysX(float a_deltaTime);
	//Adds a physics box to the scene. a_visible determines whether or not the box should also be added to the renderer.
	void AddBox(PxMaterial* const a_material, const float a_density, const vec3& a_dimensions, const vec3& a_position, const bool a_visible);
	//Adds a physics sphere to the scene. a_visible determines whether or not the sphere should also be added to the renderer.
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

	//The foundation used for managing physX.
	PxFoundation* g_physicsFoundation;
	//The physics world used.
	PxPhysics* g_physics;
	//The physics scene used for the current scene.
	PxScene* g_physicsScene;
	//The error callback PhysX will use.
	PxDefaultErrorCallback g_defaultErrorCallback;
	//The allocator callback PhysX will use.
	PxAllocatorCallback* g_allocatorCallback;
	// The filter shader PhysX will use.
	PxSimulationFilterShader g_defaultFilterShader = PxDefaultSimulationFilterShader;
	//The connection to the PhysX Visual Debugger physX will use.
	PxVisualDebuggerConnection* g_pvdConnection;
	//The cpu dispatcher PhysX will use.
	PxCpuDispatcher* g_cpuDispatcher;

	//The PhysX object that is being used for terrain.
	PxRigidStatic* g_terrain;

	//Vector used to store all of the dynamic actors within the scene. Each physics object in this vector corresponds to the renderer handle at the same index in the m_models vector.
	std::vector<PxRigidDynamic*> g_physicsActors;
	//Vector used to store all of the cloth objects within the scene. Each physics object in this vector corresponds to the renderer handle at the same index in the m_clothModels vector.
	std::vector<PxCloth*> g_physicsCloths;

	//Vector used to store renderer handles to each of the models used within the scene. Each handle in this vector corresponds to the physics object at the same index in the g_physicsActors vector.
	std::vector<unsigned int> m_models;
	//The scale that each model should be at.
	std::vector<vec3> m_scales;
	//Vector used to store renderer handles to each of the models used to represent cloth within the scene. Each handle in this vector corresponds to the physics cloth at the same index in the g_physicsCloths vector.
	std::vector<unsigned int> m_clothModels;

	//The camera used within the scene.
	WalkCamera* m_camera;
	//The renderer used to render the scene.
	Renderer* m_renderer;
};

#endif