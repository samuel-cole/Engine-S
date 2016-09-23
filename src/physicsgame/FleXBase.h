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

#include "glm\glm.hpp"
#include "glm\ext.hpp"



class FlyCamera;
class Renderer;
struct FlexSolver;
struct FlexExtAsset;

using glm::vec4;
using glm::vec3;
using glm::quat;

class FleXBase : public Application
{
protected:
	virtual int Init() override;
	virtual int Deinit() override;
	//Update contains the FleXFence stuff required for async data transfers.
	//As such, any functions modifying/accessing flex should happen BEFORE update in deriving classes,
	//then any functions that use that data (eg: updating render data) should happen AFTER update in deriving classes.
	virtual void Update(float a_deltaTime) override;
	virtual void Draw() override;

	//The camera used within the scene.
	FlyCamera* m_camera;
	//The renderer used to render the scene.
	Renderer* m_renderer;

	FlexSolver* m_solver;

	//TODO: Modify add cloth to provide more control over the cloth that is spawned (position, rotation, static points, etc.)
	//TODO: Modify add cloth to support multiple cloths, and to support scenes in which the cloth is not created first.
	//Adds a cloth with the specified mesh dimensions. a_tetherIndices specifies which particles should be used as tethers (not use physics).
	void AddCloth(unsigned int a_dimensions, unsigned int a_numberOfTethers, unsigned int* a_tetherIndices);
	void AddBox(vec3 a_position, quat a_rotation);

	float m_particleRadius;

	std::vector<unsigned int> m_clothModels;
	std::vector<unsigned int> m_boxModels;
	std::vector<FlexExtAsset*> g_cloths;
	std::vector<FlexExtAsset*> g_cubes;

	std::vector<int> m_shapeOffsets;
	std::vector<int> m_shapeIndices;
	std::vector<float> m_restPositions;
	std::vector<float> m_shapeCoefficients;
	std::vector<vec3> m_positions;
	std::vector<quat> m_rotations;

	//The triangle indices used within cloth within the game.
	std::vector<int> m_clothIndices;
	//Index to which particle within the m_particles array represents the first particle for each piece of cloth.
	std::vector<int> m_clothParticleStartIndices;

	int m_numberOfParticles;
	int m_numberOfActiveParticles;

	unsigned int m_currentHighestPhase;

	float* m_particles;
	float* m_velocities;
	int* m_phases;

	int* m_activeParticles;


	// calculates local space positions given a set of particles and rigid indices
	// Copy + pasted from FleX demo code and modified to use glm vectors and return normals.
	void CalculateRigidOffsets(const vec4* restPositions, const int* offsets, const int* indices, int numRigids, vec3* localPositions, vec4* normals);
};


#endif