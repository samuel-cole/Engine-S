#ifndef FLEX_BASE_H
#define FLEX_BASE_H

#include "Application.h"

#include <cassert>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <numeric>

#include <flex.h>

#include "glm\glm.hpp"
#include "glm\ext.hpp"

class TrackerCamera;
class Renderer;
struct FlexSolver;
struct FlexExtAsset;

using glm::vec4;
using glm::vec3;
using glm::quat;

//Base class for FleX applications- handles wrapping FleX functions for simple tasks, and managing general boilerplate code that it necessary for any FleX project.
class FleXBase : public Application
{
	//TODO: This class has a lot of containers that should always be the same size as each other (eg: there should be a rendering handle for each physics handle).
	//Consider making new classes/structs and changing these to use them so that vectors that are required to be the same size are replaced by a single vector of a new type.
	//This may also hurt caching by preventing data of the same type from being contiguous in memoery though, so look into it further before doing.


public:
	//Each of these functions returns the index of the object they have created within the appropriate vector.

	//TODO: Modify add cloth to provide more control over the cloth that is spawned (position, rotation, etc.)
	//Adds a cloth with the specified mesh dimensions. a_tetherIndices specifies which particles should be used as tethers (not use physics).
	unsigned int AddCloth(unsigned int a_dimensions, unsigned int a_numberOfTethers, unsigned int* a_tetherIndices, float a_height);
	//Adds a physics box at the specified position and with the specified rotation.
	unsigned int AddBox(vec3 a_position, quat a_rotation);
	//Adds a static sphere at the specified position, with the specified radius. 
	//a_isTrigger is used to specify whether or not this sphere should be a trigger object (entities pass through it) or not.
	unsigned int AddStaticSphere(float a_radius, vec3 a_position, bool a_isTrigger);
	//Adds fluid to the scene. NOTE: in order for this to work, the physics properties from the scene must be set up to handle fluids.
	//The dimension variables are used for specifying how many particles will be created, and where they'll be in relation to a_lower.
	void AddFluid(vec3 a_lower, int a_dimX, int a_dimY, int a_dimZ, int a_phaseNumber);


protected:
	//Called when the game starts. Sets up the physics properties for the scene.
	virtual int Init() override;
	//Called at the end of the game, frees all of the memory allocated by the class.
	virtual int Deinit() override;
	//Update handles updating FleX, and ensuring that all render objects remain in sync with their physics counterparts.
	//NOTE: Update contains the FleXFence calls required for async data transfers.
	//As such, any functions modifying/accessing flex should happen BEFORE update in deriving classes,
	//then any functions that use that data (eg: updating render data) should happen AFTER update in deriving classes.
	virtual void Update(float a_deltaTime) override;
	//Handles telling the renderer to draw the scene.
	virtual void Draw() override;

	//Destroy All removes all objects from the game, however doesn't remove the assets present in every scene (eg: renderer, camera, etc.)
	void DestroyAll();

	//The camera used within the scene.
	TrackerCamera* m_camera;
	//The renderer used to render the scene.
	Renderer* m_renderer;
	//The physics solver used within the scene.
	FlexSolver* g_solver;

	//The radius that FleX particles should have. Used in various calculations related to radius (eg: some variables need to be set to half the particle radius, or etc.)
	float m_particleRadius;

	//======================
	//Static shape variables
	//======================
	//Rendering handles for each static shape in the scene.
	std::vector<unsigned int> m_shapeModels;
	//FleX storage for shape-specific information for each static shape in the scene.
	std::vector<FlexCollisionGeometry> g_shapeGeometry;
	//Positions for each static shape in the scene.
	std::vector<vec4> m_shapePositions;
	//Rotations for each static shape in the scene.
	std::vector<quat> m_shapeRotations;
	//Vector of offsets into g_shapeGeometry for each shape- because each shape currently only has one piece of geometry attached, each entry in this list should just be the same as its index within this vector.
	std::vector<uint32_t> m_shapeStarts;
	//Vector containing the flags defining the behaviour for each shape (eg: trigger or not)
	std::vector<int> m_shapeFlags;
	//Vector containing the positions of each shape's AABB minimum position, in world space.
	std::vector<vec4> m_shapeAABBmins;
	//Vector containing the positions of each shape's AABB maximum position, in world space.
	std::vector<vec4> m_shapeAABBmaxes;

	//====================
	//Rigidbody variables.
	//====================
	//Rendering handles for each cube object in the scene.
	std::vector<unsigned int> m_boxModels;
	//FleX handles for each cube in the scene.
	std::vector<FlexExtAsset*> g_cubes;
	std::vector<int> m_rigidOffsets;
	std::vector<int> m_rigidIndices;
	std::vector<float> m_rigidRestPositions;
	std::vector<float> m_rigidCoefficients;
	std::vector<vec3> m_rigidPositions;
	std::vector<quat> m_rigidRotations;

	//======================
	//Cloth/spring variables
	//======================
	//Rendering handles for each cloth object in the scene.
	std::vector<unsigned int> m_clothModels;
	//FleX handles for each cloth in the scene.
	std::vector<FlexExtAsset*> g_cloths;
	//The triangle indices used within cloth within the game.
	std::vector<int> m_clothIndices;
	//Index to which particle within the m_particles array represents the first particle for each piece of cloth.
	std::vector<int> m_clothParticleStartIndices;
	std::vector<int> m_springIndices;
	std::vector<float> m_springRestLengths;
	std::vector<float> m_springStiffness;

	//Vector of indices to the particle/velocity/phase arrays for where each fluid is.
	std::vector<unsigned int> m_fluidParticles;
	//Placeholder- this currently holds render handles to a lot of cubes, which are used for rendering fluid particles.
	std::vector<unsigned int> m_fluidRenderHandles;

	int m_numberOfParticles;
	int m_numberOfClothParticles;
	int m_numberOfActiveParticles;
	int m_numberOfFluidParticles;

	unsigned int m_currentHighestPhase;

	float* m_particles;
	float* m_velocities;
	int* m_phases;

	int* m_activeParticles;

	bool m_updateFleXScene;

	// calculates local space positions given a set of particles and rigid indices
	// Copy + pasted from FleX demo code and modified to use glm vectors, return normals, and to use indices that don't include cloth indices.
	void CalculateRigidOffsets(const vec4* restPositions, const int* offsets, const int* indices, int numRigids, vec3* localPositions, vec4* normals);
};


#endif