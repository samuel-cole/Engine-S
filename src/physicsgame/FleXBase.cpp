#include "FleXBase.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "glm\gtx\euler_angles.hpp"
#include "InputManager.h"

#include <flex.h>
#include <flexExt.h>

int FleXBase::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);

	m_numberOfParticles = 10000;
	m_numberOfClothParticles = 0;
	m_numberOfActiveParticles = 0;
	int sqrtNumberOfParticles = glm::sqrt(m_numberOfParticles);

	m_particleRadius = 0.5f;

	flexInit();
	g_solver = flexCreateSolver(m_numberOfParticles, 0);

	m_currentHighestPhase = 0;

	FlexParams params;
	params.mGravity[0] = 0.0f;
	params.mGravity[1] = -9.8f;
	params.mGravity[2] = 0.0f;

	//=====================================
	//Taken from flex demo.
	params.mWind[0] = 0.0f;
	params.mWind[1] = 0.0f;
	params.mWind[2] = 0.0f;
	
	params.mRadius = m_particleRadius;
	params.mViscosity = 0.0f;
	params.mDynamicFriction = 0.25f;
	params.mStaticFriction = 0.0f;
	params.mParticleFriction = 0.0f; // scale friction between particles by default
	params.mFreeSurfaceDrag = 0.0f;
	params.mDrag = 0.0f;
	params.mLift = 0.0f;
	params.mNumIterations = 3;
	params.mFluidRestDistance = 0.0f;
	params.mSolidRestDistance = 0.0f;
	
	params.mAnisotropyScale = 1.0f;
	params.mAnisotropyMin = 0.1f;
	params.mAnisotropyMax = 2.0f;
	params.mSmoothing = 1.0f;
	
	params.mDissipation = 0.0f;
	params.mDamping = 0.0f;
	params.mParticleCollisionMargin = 0.0f;
	params.mShapeCollisionMargin = 0.0f;
	params.mCollisionDistance = 0.05f;
	params.mPlasticThreshold = 0.0f;
	params.mPlasticCreep = 0.0f;
	params.mFluid = false;
	params.mSleepThreshold = 0.0f;
	params.mShockPropagation = 0.0f;
	params.mRestitution = 0.0f;
	params.mMaxSpeed = FLT_MAX;
	params.mRelaxationMode = eFlexRelaxationLocal;
	params.mRelaxationFactor = 0.0f;
	params.mSolidPressure = 1.0f;
	params.mAdhesion = 0.0f;
	params.mCohesion = 0.025f;
	params.mSurfaceTension = 0.0f;
	params.mVorticityConfinement = 0.0f;
	params.mBuoyancy = 1.0f;
	params.mDiffuseThreshold = 100.0f;
	params.mDiffuseBuoyancy = 1.0f;
	params.mDiffuseDrag = 0.8f;
	params.mDiffuseBallistic = 16;
	params.mDiffuseSortAxis[0] = 0.0f;
	params.mDiffuseSortAxis[1] = 0.0f;
	params.mDiffuseSortAxis[2] = 0.0f;
	params.mDiffuseLifetime = 2.0f;
	params.mInertiaBias = 0.001f;
	
	//planes created after particles
	params.mNumPlanes = 5;
	
	//Values from FleX demo taken, but code isn't taken directly.
	params.mSolidRestDistance = params.mRadius;
	params.mCollisionDistance = params.mRadius;
	params.mParticleFriction = params.mDynamicFriction * 0.1f;
	params.mShapeCollisionMargin = params.mCollisionDistance * 0.5f;
	(vec4&)params.mPlanes[0] = vec4(0, 1, 0, 0);
	(vec4&)params.mPlanes[1] = vec4(0.0f, 0.0f, 1.0f,  30);
	(vec4&)params.mPlanes[2] = vec4(1.0f, 0.0f, 0.0f,  30);
	(vec4&)params.mPlanes[3] = vec4(-1.0f, 0.0f, 0.0f, 30);
	(vec4&)params.mPlanes[4] = vec4(0.0f, 0.0f, -1.0f, 30);

	unsigned int* planes = new unsigned int[5];
	for (int i = 0; i < 5; ++i)
	{
		planes[i] = m_renderer->GenerateGrid(100, 100);
		m_renderer->LoadAmbient("../data/tablecloth.jpg", planes[i]);
		m_renderer->LoadTexture("../data/tablecloth.jpg", planes[i]);
		m_renderer->LoadSpecularMap("../data/tablecloth.jpg", planes[i]);
	}

	m_renderer->SetPosition(vec3(30, 0, 0), planes[1]);
	m_renderer->SetRotation(quat(vec3(0, 0, glm::pi<float>()/2.0f)), planes[1]);
	m_renderer->SetPosition(vec3(0, 0, 30), planes[2]);
	m_renderer->SetRotation(quat(vec3(glm::pi<float>()/2.0f, 0, 0)), planes[2]);
	m_renderer->SetPosition(vec3(0, 0, -30), planes[3]);
	m_renderer->SetRotation(quat(vec3(-glm::pi<float>() / 2.0f, 0, 0)), planes[3]);
	m_renderer->SetPosition(vec3(-30, 0, 0), planes[4]);
	m_renderer->SetRotation(quat(vec3(0, 0, -glm::pi<float>() / 2.0f)), planes[4]);

	m_particles = new float[m_numberOfParticles * 4];
	m_velocities = new float[m_numberOfParticles * 3];
	m_phases = new int[m_numberOfParticles];

	m_activeParticles = new int[m_numberOfParticles];

	//Shape offsets should always start with just a 0 in it.
	m_rigidOffsets.push_back(0);

	flexSetParams(g_solver, &params);

	return 0;
}

int FleXBase::Deinit()
{
	delete[] m_particles;
	delete[] m_velocities;
	delete[] m_phases;

	delete[] m_activeParticles;

	flexDestroySolver(g_solver);
	flexShutdown();

	m_renderer->CleanupBuffers();
	delete m_renderer;
	delete m_camera;

	return Application::Deinit();
}

void FleXBase::Update(float a_deltaTime)
{
	m_camera->Update(a_deltaTime);

	//These are only needed if particles are being moved manually- in most situations, these should be commented out.
	flexSetParticles(g_solver, m_particles, m_numberOfActiveParticles, eFlexMemoryHostAsync);
	flexSetVelocities(g_solver, m_velocities, m_numberOfActiveParticles, eFlexMemoryHostAsync);

	if (InputManager::GetKey(Keys::SPACE))
		flexUpdateSolver(g_solver, 1.0f / 60.0f, 2, NULL);

	flexGetParticles(g_solver, m_particles, m_numberOfParticles, eFlexMemoryHostAsync);
	flexGetVelocities(g_solver, m_velocities, m_numberOfParticles, eFlexMemoryHostAsync);

	if (m_rigidRotations.size() > 0)
		flexGetRigidTransforms(g_solver, (float*)&m_rigidRotations[0], (float*)&m_rigidPositions[0], eFlexMemoryHostAsync);

	flexSetFence();
	flexWaitFence();

	for (unsigned int i = 0; i < g_cloths.size(); ++i)
	{
		std::vector<vec3> particlePositions;
		for (int j = m_clothParticleStartIndices[i]; j < m_clothParticleStartIndices[i] + g_cloths[i]->mNumParticles; ++j)
		{
			particlePositions.push_back((vec3&)m_particles[j * 4]);
		}
		m_renderer->ModifyMesh(m_clothModels[i], particlePositions);
	}

	for (unsigned int i = 0; i < g_cubes.size(); ++i)
	{
		m_renderer->SetPosition(m_rigidPositions[i], m_boxModels[i]);
		m_renderer->SetRotation(m_rigidRotations[i], m_boxModels[i]);
	}
}

void FleXBase::Draw()
{
	m_renderer->Draw();
}

void FleXBase::AddCloth(unsigned int a_dimensions, unsigned int a_numberOfTethers, unsigned int* a_tetherIndices, float a_height)
{
	unsigned int numberOfVertices, numberOfIndices = -1;
	float* vertices = nullptr;
	int* indices = nullptr;

	m_clothModels.push_back(m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1, m_particleRadius, a_height, numberOfVertices, vertices, numberOfIndices, indices));

	for (unsigned int i = 0; i < a_numberOfTethers; ++i)
	{
		vertices[a_tetherIndices[i] * 4 + 3] = 0;
	}

	unsigned int numberOfTriangles = numberOfIndices / 3;
	//Flex says that the indices should be passed through the flexExtCreateWeldedMeshIndices function, consider trying this.
	FlexExtAsset* g_cloth = flexExtCreateClothFromMesh(vertices, numberOfVertices, indices, numberOfTriangles, 0.9f, 0.8f, 0.5f, 5.0f, 0.0f);

	int phase = flexMakePhase(m_currentHighestPhase++, eFlexPhaseSelfCollide);

	for (int i = m_numberOfActiveParticles; i < m_numberOfActiveParticles + g_cloth->mNumParticles; ++i)
	{
		int indexInCurrentCloth = i - m_numberOfActiveParticles;

		m_activeParticles[i] = i;

		m_particles[i * 4 + 0] = g_cloth->mParticles[indexInCurrentCloth * 4 + 0];
		m_particles[i * 4 + 1] = g_cloth->mParticles[indexInCurrentCloth * 4 + 1];
		m_particles[i * 4 + 2] = g_cloth->mParticles[indexInCurrentCloth * 4 + 2];
		m_particles[i * 4 + 3] = g_cloth->mParticles[indexInCurrentCloth * 4 + 3];

		m_velocities[i * 3 + 0] = 0;
		m_velocities[i * 3 + 1] = 0;
		m_velocities[i * 3 + 2] = 0;

		m_phases[i] = phase;
	}
	for (unsigned int i = 0; i < numberOfIndices; ++i)
	{
		m_clothIndices.push_back(indices[i]);
	}
	for (int i = 0; i < g_cloth->mNumSprings; ++i)
	{
		m_springIndices.push_back(g_cloth->mSpringIndices[i * 2] + m_numberOfActiveParticles);
		m_springIndices.push_back(g_cloth->mSpringIndices[i * 2 + 1] + m_numberOfActiveParticles);
		m_springRestLengths.push_back(g_cloth->mSpringRestLengths[i]);
		m_springStiffness.push_back(g_cloth->mSpringCoefficients[i]);
	}

	m_clothParticleStartIndices.push_back(m_numberOfActiveParticles);
	m_numberOfActiveParticles += numberOfVertices;
	m_numberOfClothParticles += numberOfVertices;

	flexSetParticles(g_solver, m_particles, m_numberOfActiveParticles, eFlexMemoryHostAsync);
	flexSetVelocities(g_solver, m_velocities, m_numberOfActiveParticles, eFlexMemoryHostAsync);
	flexSetPhases(g_solver, m_phases, m_numberOfActiveParticles, eFlexMemoryHostAsync);
	flexSetActive(g_solver, m_activeParticles, m_numberOfActiveParticles, eFlexMemoryHostAsync);

	//This won't work with multiple cloths.
	flexSetSprings(g_solver, &m_springIndices[0], &m_springRestLengths[0], &m_springStiffness[0], m_springStiffness.size(), eFlexMemoryHostAsync);
	flexSetDynamicTriangles(g_solver, &m_clothIndices[0], NULL, m_clothIndices.size() / 3, eFlexMemoryHostAsync);

	delete[] vertices;
	delete[] indices;

	g_cloths.push_back(g_cloth);
}

void FleXBase::AddBox(vec3 a_position, quat a_rotation)
{
	unsigned int numberOfVertices, numberOfIndices = -1;
	float* vertices = nullptr;
	int* indices = nullptr;

	m_rigidPositions.push_back(a_position);
	m_rigidRotations.push_back(a_rotation);

	unsigned int model = m_renderer->LoadOBJ("../data/cube.obj", numberOfVertices, vertices, numberOfIndices, indices);
	m_boxModels.push_back(model);
	m_renderer->SetPosition(a_position, model);
	m_renderer->SetRotation(a_rotation, model);

	FlexExtAsset* g_cube = flexExtCreateRigidFromMesh(vertices, numberOfVertices, indices, numberOfIndices, m_particleRadius, 0.0f);

	mat4 transform = m_renderer->GetTransform(model);
	int phase = flexMakePhase(m_currentHighestPhase++, 0);
	for (int i = m_numberOfActiveParticles; i < m_numberOfActiveParticles + g_cube->mNumParticles; ++i)
	{
		int indexInCurrentCube = i - m_numberOfActiveParticles;
		m_phases[i] = phase;
		m_activeParticles[i] = i;

		float x = g_cube->mParticles[indexInCurrentCube * 4 + 0];
		float y = g_cube->mParticles[indexInCurrentCube * 4 + 1];
		float z = g_cube->mParticles[indexInCurrentCube * 4 + 2];

		//The idea behind doing this transform is that at the moment, the object is always being spawned at world origin, regardless of the position/rotation passed in.
		//This is because even though position/rotation is passed into the function for making a rigidbody, the particles themselves aren't set to the correct positions.
		vec4 vertex = transform * vec4(x, y, z, 1);

		m_particles[i * 4 + 0] = vertex.x;
		m_particles[i * 4 + 1] = vertex.y;
		m_particles[i * 4 + 2] = vertex.z;
		m_particles[i * 4 + 3] = g_cube->mParticles[indexInCurrentCube * 4 + 3];

		m_velocities[i * 3 + 0] = 0;
		m_velocities[i * 3 + 1] = 0;
		m_velocities[i * 3 + 2] = 0;

		m_rigidRestPositions.push_back(x);
		m_rigidRestPositions.push_back(y);
		m_rigidRestPositions.push_back(z);
		m_rigidRestPositions.push_back(1);

		m_rigidIndices.push_back(i);
	}

	m_numberOfActiveParticles += g_cube->mNumParticles;

	m_rigidOffsets.push_back(m_rigidOffsets[m_rigidOffsets.size() - 1] + g_cube->mShapeOffsets[0]);

	vec3* cubeLocalRestPositions = new vec3[m_rigidIndices.size()];
	vec4* cubeLocalNormals = new vec4[m_rigidIndices.size()];

	CalculateRigidOffsets((vec4*)&m_rigidRestPositions[0], &m_rigidOffsets[0], &m_rigidIndices[0], g_cubes.size() + 1, cubeLocalRestPositions, cubeLocalNormals);

	flexSetParticles(g_solver, m_particles, m_numberOfParticles, eFlexMemoryHostAsync);
	flexSetVelocities(g_solver, m_velocities, m_numberOfParticles, eFlexMemoryHostAsync);
	flexSetPhases(g_solver, m_phases, m_numberOfParticles, eFlexMemoryHostAsync);
	flexSetActive(g_solver, m_activeParticles, m_numberOfActiveParticles, eFlexMemoryHostAsync);

	m_rigidCoefficients.push_back(g_cube->mShapeCoefficients[0]);

	flexSetRigids(g_solver, &m_rigidOffsets[0], &m_rigidIndices[0], (float*)cubeLocalRestPositions, (float*)cubeLocalNormals, &m_rigidCoefficients[0], (float*)&m_rigidRotations[0], (float*)&m_rigidPositions[0], g_cubes.size() + 1, eFlexMemoryHostAsync);
	
	delete[] cubeLocalRestPositions;
	delete[] vertices;
	delete[] indices;

	g_cubes.push_back(g_cube);
}

void FleXBase::AddShape()
{
	//TODO: fill out the following information:
	//g_shapeGeometry
	//m_shapeStarts
	//m_shapePositions
	//m_shapeRotations
	//m_shapeFlags
	//(maybe)AABB stuff (also consider replacing nullptr with NULL)


	//Update notes imply that AABB min/maxes aren't necessary?
	//flexSetShapes(g_solver, &g_shapeGeometry[0], (int)g_shapeGeometry.size(), nullptr, nullptr, (int*)&m_shapeStarts[0], (float*)&m_shapePositions[0], (float*)&m_shapeRotations[0], (float*)&m_shapePositions[0], (float*)&m_shapeRotations[0], &m_shapeFlags[0], (int)m_shapeStarts.size(), eFlexMemoryHostAsync);
					
}


// Copy + pasted from FleX demo code and modified to use glm vectors, return normals, and to use indices that don't include cloth indices.
//This function may not be needed, its end result is just the same as the starting position, as the particles are still at world 0 when this is called.
void FleXBase::CalculateRigidOffsets(const vec4* restPositions, const int* offsets, const int* indices, int numRigids, vec3* localPositions, vec4* normals)
{
	int count = 0;

	for (int r = 0; r < numRigids; ++r)
	{
		const int startIndex = offsets[r];
		const int endIndex = offsets[r + 1];

		const int n = endIndex - startIndex;

		assert(n);

		vec3 com;

		for (int i = startIndex; i < endIndex; ++i)
		{
			const int r = indices[i] - m_numberOfClothParticles;

			com += vec3(restPositions[r]);
		}

		com /= float(n);

		for (int i = startIndex; i < endIndex; ++i)
		{
			const int r = indices[i] - m_numberOfClothParticles;

			vec3 position = vec3(restPositions[r]) - com;
			float distance = glm::length(position);
			localPositions[count] = position;
			//normal uses position, but normalized, and with the negative distance as the 4th component.
			normals[count++] = vec4((1.0f / distance) * position, -distance);
		}
	}
}