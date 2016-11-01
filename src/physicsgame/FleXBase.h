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

//=======================================================
//FleX project addition
//=======================================================
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
	//Vector used for telling FleX which rigid indices are used for each rigidbody- the i'th rigid body will use all of the rigid indices from m_rigidOffsets[i] to m_rigidOffsets[i + 1].
	std::vector<int> m_rigidOffsets;
	//Vector containing each particle used in a rigid body's index within the main particle array.
	std::vector<int> m_rigidIndices;
	//The default positions of each particle within its rigid body (before that rigid body has moved at all). Should be 4 times the number of particles in length, in the format (x, y, z, inverse mass)
	std::vector<float> m_rigidRestPositions;
	//The stiffness coefficient of each rigidbody. Given that the renderer doesn't currently support deformation of rigid bodies, this should always be filled with 1's.
	std::vector<float> m_rigidCoefficients;
	//The positions of each rigid body.
	std::vector<vec3> m_rigidPositions;
	//The rotations of each rigid body.
	std::vector<quat> m_rigidRotations;

	//======================
	//Cloth/spring variables
	//NOTE: at the moment, springs aren't used for anything except cloth.
	//======================
	//Rendering handles for each cloth object in the scene.
	std::vector<unsigned int> m_clothModels;
	//FleX handles for each cloth in the scene.
	std::vector<FlexExtAsset*> g_cloths;
	//The triangle indices used within cloth within the game.
	std::vector<int> m_clothIndices;
	//Index to which particle within the m_particles array represents the first particle for each piece of cloth.
	std::vector<int> m_clothParticleStartIndices;
	//Set of indices into the main particle array for which particles should be connected by springs. Should always be twice the number of springs in length, specified in pairs.
	std::vector<int> m_springIndices;
	//The length that each spring will attempt to restore itself to.
	std::vector<float> m_springRestLengths;
	//The stiffness of each spring.
	std::vector<float> m_springStiffness;

	//================
	//Fluid variables
	//================
	//Vector of indices to the particle/velocity/phase arrays for where each fluid is.
	std::vector<unsigned int> m_fluidParticles;
	//Render handles for each sphere used for rendering fluid particles.
	std::vector<unsigned int> m_fluidRenderHandles;

	//The maximum number of particles for the scene.
	int m_numberOfParticles;
	//The number of particles that are currently in the scene.
	int m_numberOfActiveParticles;
	//The number of cloth particles that are currently in the scene.
	int m_numberOfClothParticles;
	//The number of fluid particles that are currently in the scene.
	int m_numberOfFluidParticles;

	//Number used for giving each object a different collision phase- is incremented for each new object, except in special cases where that new object shouldn't collide with objects (such as fluid).
	//Doesn't contain the extra information used by FleX for phases, such as flags for self colliding objects or fluids.
	unsigned int m_currentHighestPhase;

	//The main array used for storing particles within the scene. Is set to four times the number of particles in length, and is specified in the format x, y, z, inverse mass.
	float* m_particles;
	//The array used for storing the velocity of each particle within the scene. Is set to three times the number of particles in length, and is specified in the format x, y, z.
	float* m_velocities;
	//The array used for storing the phase of each particle within the scene. Is set to the number of particles in length. 
	//Each phase stores it's collision group in the lower 24 bits of its int, and extra collision flags in the rest.
	int* m_phases;

	//Array of indices to the main particle array, used for specifying which particles should be active.
	int* m_activeParticles;

	//Setting for whether the physics scene should be being updated or not- set to false to pause physics simulation.
	bool m_updateFleXScene;

	// Calculates local space positions given a set of particles and rigid indices
	// Copy + pasted from FleX demo code and modified to use glm vectors, return normals, and to use indices that don't include cloth indices.
	void CalculateRigidOffsets(const vec4* restPositions, const int* offsets, const int* indices, int numRigids, vec3* localPositions, vec4* normals);
};


#endif