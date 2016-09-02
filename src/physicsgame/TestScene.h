#ifndef TESTSCENE_H
#define TESTSCENE_H

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

//struct FlexParticle
//{
//	vec3 position;
//	vec3 velocity;
//	float inverseMass;
//	int phase;
//}

class TestScene : public Application
{
protected:
	virtual int Init() override;
	virtual int Deinit() override;
	virtual void Update(float a_deltaTime) override;
	virtual void Draw() override;

	//The camera used within the scene.
	FlyCamera* m_camera;
	//The renderer used to render the scene.
	Renderer* m_renderer;

	FlexSolver* m_solver;

	void AddCloth(unsigned int a_dimensions);
	void AddBox(vec3 a_position, glm::quat a_rotation);

	float timeInScene;

	unsigned int m_clothModel;
	FlexExtAsset* g_cloth;

	int m_numberOfParticles;

	float* m_particles;
	float* m_velocities;
	int* m_phases;

	int* m_activeParticles;
	int* m_indices;
	float* m_verticies;

	vec3 m_gravityDir;
	float m_gravityStrength;

	vec3 m_oldgravityDir;
	float m_oldGravityStrength;

	// calculates local space positions given a set of particles and rigid indices
	// Copy + pasted from FleX demo code and modified to use glm vectors instead.
	void CalculateRigidOffsets(const vec4* restPositions, const int* offsets, const int* indices, int numRigids, vec3* localPositions);
};


#endif