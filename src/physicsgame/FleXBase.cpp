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
	m_numberOfActiveParticles = 0;
	int sqrtNumberOfParticles = glm::sqrt(m_numberOfParticles);

	m_particleRadius = 0.5f;

	flexInit();
	m_solver = flexCreateSolver(m_numberOfParticles, 0);

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
	params.mDynamicFriction = 0.0f;
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
	params.mCollisionDistance = 0.0f;
	params.mPlasticThreshold = 0.0f;
	params.mPlasticCreep = 0.0f;
	params.mFluid = false;
	params.mSleepThreshold = 0.0f;
	params.mShockPropagation = 0.0f;
	params.mRestitution = 0.0f;
	params.mMaxSpeed = FLT_MAX;
	params.mRelaxationMode = eFlexRelaxationLocal;
	params.mRelaxationFactor = 1.0f;
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
	m_shapeOffsets.push_back(0);

	flexSetParams(m_solver, &params);

	return 0;
}

int FleXBase::Deinit()
{
	delete[] m_particles;
	delete[] m_velocities;
	delete[] m_phases;

	delete[] m_activeParticles;

	flexDestroySolver(m_solver);
	flexShutdown();

	m_renderer->CleanupBuffers();
	delete m_renderer;
	delete m_camera;

	return Application::Deinit();
}

void FleXBase::Update(float a_deltaTime)
{
	m_camera->Update(a_deltaTime);

	if (InputManager::GetKey(Keys::SPACE))
		flexUpdateSolver(m_solver, 1.0f/60.0f, 1, NULL);

	flexGetParticles(m_solver, m_particles, m_numberOfParticles, eFlexMemoryHostAsync);
	flexGetVelocities(m_solver, m_velocities, m_numberOfParticles, eFlexMemoryHostAsync);

	if (m_rotations.size() > 0)
		flexGetRigidTransforms(m_solver, (float*)&m_rotations[0], (float*)&m_positions[0], eFlexMemoryHostAsync);

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
		m_renderer->SetPosition(m_positions[i], m_boxModels[i]);
		m_renderer->SetRotation(m_rotations[i], m_boxModels[i]);
	}
}

void FleXBase::Draw()
{
	m_renderer->Draw();
}

void FleXBase::AddCloth(unsigned int a_dimensions, unsigned int a_numberOfTethers, unsigned int* a_tetherIndices)
{
	unsigned int numberOfVertices, numberOfIndices = -1;
	float* vertices = nullptr;
	int* indices = nullptr;

	m_clothModels.push_back(m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1, 10.0f, numberOfVertices, vertices, numberOfIndices, indices));

	for (unsigned int i = 0; i < a_numberOfTethers; ++i)
	{
		vertices[a_tetherIndices[i] * 4 + 3] = 0;
	}

	unsigned int numberOfTriangles = numberOfIndices / 3;
	//Flex says that the indices should be passed through the flexExtCreateWeldedMeshIndices function, consider trying this.
	FlexExtAsset* g_cloth = flexExtCreateClothFromMesh(vertices, numberOfVertices, indices, numberOfTriangles, 0.9f, 1.0f, 1.0f, 5.0f, 0.0f);

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

	m_clothParticleStartIndices.push_back(m_numberOfActiveParticles);
	m_numberOfActiveParticles += numberOfVertices;

	flexSetParticles(m_solver, m_particles, m_numberOfActiveParticles, eFlexMemoryHostAsync);
	flexSetVelocities(m_solver, m_velocities, m_numberOfActiveParticles, eFlexMemoryHostAsync);
	flexSetPhases(m_solver, m_phases, m_numberOfActiveParticles, eFlexMemoryHostAsync);
	flexSetActive(m_solver, m_activeParticles, m_numberOfActiveParticles, eFlexMemoryHostAsync);

	flexSetSprings(m_solver, g_cloth->mSpringIndices, g_cloth->mSpringRestLengths, g_cloth->mSpringCoefficients, g_cloth->mNumSprings, eFlexMemoryHostAsync);

	flexSetDynamicTriangles(m_solver, &m_clothIndices[0], NULL, m_clothIndices.size() / 3, eFlexMemoryHostAsync);

	delete[] vertices;
	delete[] indices;

	g_cloths.push_back(g_cloth);
}

void FleXBase::AddBox(vec3 a_position, quat a_rotation)
{
	unsigned int numberOfVertices, numberOfIndices = -1;
	float* vertices = nullptr;
	int* indices = nullptr;

	m_positions.push_back(a_position);
	m_rotations.push_back(a_rotation);

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

		m_restPositions.push_back(x);
		m_restPositions.push_back(y);
		m_restPositions.push_back(z);
		m_restPositions.push_back(1);

		m_shapeIndices.push_back(i);
	}

	m_numberOfActiveParticles += g_cube->mNumParticles;

	m_shapeOffsets.push_back(m_shapeOffsets[m_shapeOffsets.size() - 1] + g_cube->mShapeOffsets[0]);

	vec3* cubeLocalRestPositions = new vec3[m_shapeIndices.size()];
	vec4* cubeLocalNormals = new vec4[m_shapeIndices.size()];

	CalculateRigidOffsets((vec4*)&m_restPositions[0], &m_shapeOffsets[0], &m_shapeIndices[0], g_cubes.size() + 1, cubeLocalRestPositions, cubeLocalNormals);

	flexSetParticles(m_solver, m_particles, m_numberOfParticles, eFlexMemoryHostAsync);
	flexSetVelocities(m_solver, m_velocities, m_numberOfParticles, eFlexMemoryHostAsync);
	flexSetPhases(m_solver, m_phases, m_numberOfParticles, eFlexMemoryHostAsync);
	flexSetActive(m_solver, m_activeParticles, m_numberOfActiveParticles, eFlexMemoryHostAsync);

	m_shapeCoefficients.push_back(g_cube->mShapeCoefficients[0]);

	flexSetRigids(m_solver, &m_shapeOffsets[0], &m_shapeIndices[0], (float*)cubeLocalRestPositions, (float*)cubeLocalNormals, &m_shapeCoefficients[0], (float*)&m_rotations[0], (float*)&m_positions[0], g_cubes.size() + 1, eFlexMemoryHostAsync);
	
	delete[] cubeLocalRestPositions;
	delete[] vertices;
	delete[] indices;

	g_cubes.push_back(g_cube);
}


// Copy + pasted from FleX demo code and modified to use glm vectors and return normals.
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
			const int r = indices[i];

			com += vec3(restPositions[r]);
		}

		com /= float(n);

		for (int i = startIndex; i < endIndex; ++i)
		{
			const int r = indices[i];

			vec3 position = vec3(restPositions[r]) - com;
			float distance = glm::length(position);
			localPositions[count] = position;
			//normal uses position, but normalized, and with the negative distance as the 4th component.
			normals[count++] = vec4((1.0f / distance) * position, -distance);
		}
	}
}