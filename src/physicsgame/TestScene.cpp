#include "TestScene.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "glm\gtx\euler_angles.hpp"
#include "InputManager.h"

#include <flex.h>
#include <flexExt.h>

int TestScene::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);

	m_gravityDir = vec3(0, -1, 0);
	m_gravityStrength = 9.8f;
	m_oldgravityDir = vec3(0, -1, 0);
	m_oldGravityStrength = 9.8f;

	TwAddVarRW(m_debugBar, "Gravity", TW_TYPE_DIR3F, &m_gravityDir[0], "");
	TwAddVarRW(m_debugBar, "Gravity Strength", TW_TYPE_FLOAT, &m_gravityStrength, " min=0 max=30");

	m_numberOfParticles = 2500;
	int sqrtNumberOfParticles = glm::sqrt(m_numberOfParticles);

	flexInit();
	m_solver = flexCreateSolver(m_numberOfParticles, 0);

	FlexParams params;
	params.mGravity[0] = m_gravityDir.x * m_gravityStrength;
	params.mGravity[1] = m_gravityDir.y * m_gravityStrength;
	params.mGravity[2] = m_gravityDir.z * m_gravityStrength;

	//=====================================
	//Taken from flex demo.
	params.mWind[0] = 0.0f;
	params.mWind[1] = 0.0f;
	params.mWind[2] = 0.0f;
	
	params.mRadius = 0.05f;
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

	//m_renderer->SetTransform(glm::translate(vec3(0, 0, 100)) * glm::eulerAngleYXZ(0.0f, 0.0f, glm::half_pi<float>()), planes[1]);
	m_renderer->SetPosition(vec3(30, 0, 0), planes[1]);
	m_renderer->SetRotation(glm::quat(vec3(0, 0, glm::pi<float>()/2.0f)), planes[1]);
	m_renderer->SetPosition(vec3(0, 0, 30), planes[2]);
	m_renderer->SetRotation(glm::quat(vec3(glm::pi<float>()/2.0f, 0, 0)), planes[2]);
	m_renderer->SetPosition(vec3(0, 0, -30), planes[3]);
	m_renderer->SetRotation(glm::quat(vec3(-glm::pi<float>() / 2.0f, 0, 0)), planes[3]);
	m_renderer->SetPosition(vec3(-30, 0, 0), planes[4]);
	m_renderer->SetRotation(glm::quat(vec3(0, 0, -glm::pi<float>() / 2.0f)), planes[4]);



	//======================================

	//m_particles = (float*)flexAlloc(100 * 4);
	//m_velocities = (float*)flexAlloc(100 * 3);
	//m_phases = (int*)flexAlloc(100);
	//
	//m_verticies = (float*)flexAlloc(100 * 3);
	//m_indices = (int*)flexAlloc(81 * 2 * 3);
	//m_activeParticles = (int*)flexAlloc(100);



	m_particles = new float[m_numberOfParticles * 4];
	m_velocities = new float[m_numberOfParticles * 3];
	m_phases = new int[m_numberOfParticles];

	m_verticies = new float[m_numberOfParticles * 3];
	m_indices = new int[(sqrtNumberOfParticles - 1) * (sqrtNumberOfParticles - 1) * 2 * 3];
	m_activeParticles = new int[m_numberOfParticles];

	flexSetParams(m_solver, &params);

	//AddCloth(sqrtNumberOfParticles);
	AddBox(vec3(0, 10, 0), glm::quat(vec3(0, 0, 0)));

	int version = flexGetVersion();

	timeInScene = 0.0f;

	return 0;
}

int TestScene::Deinit()
{
	//flexFree(m_particles);
	//flexFree(m_velocities);
	//flexFree(m_phases);
	//
	//flexFree(m_verticies);
	//flexFree(m_indices);
	//flexFree(m_activeParticles);

	delete[] m_particles;
	delete[] m_velocities;
	delete[] m_phases;

	delete[] m_verticies;
	delete[] m_indices;
	delete[] m_activeParticles;

	flexDestroySolver(m_solver);
	flexShutdown();

	m_renderer->CleanupBuffers();
	delete m_renderer;
	delete m_camera;

	return Application::Deinit();
}

void TestScene::Update(float a_deltaTime)
{
	timeInScene += a_deltaTime;

	//m_renderer->SetRotation(glm::quat(vec3(glm::pi<float>() * glm::sin(timeInScene), 0, 0)), 1);

	if (m_oldgravityDir != m_gravityDir || m_oldGravityStrength != m_gravityStrength)	//rewrite this to account for float equality issues.
	{
		m_oldgravityDir = m_gravityDir;
		m_oldGravityStrength = m_gravityStrength;
		FlexParams params;
		flexGetParams(m_solver, &params);
		params.mGravity[0] = m_gravityDir.x * m_gravityStrength;
		params.mGravity[1] = m_gravityDir.y * m_gravityStrength;
		params.mGravity[2] = m_gravityDir.z * m_gravityStrength;
		flexSetParams(m_solver, &params);
	}

	m_camera->Update(a_deltaTime);

	//flexSetParticles(m_solver, m_particles, 100, eFlexMemoryHostAsync);
	//flexSetVelocities(m_solver, m_velocities, 100, eFlexMemoryHostAsync);

	flexUpdateSolver(m_solver, 1.0f/60.0f, 1, NULL);

	flexGetParticles(m_solver, m_particles, m_numberOfParticles, eFlexMemoryHostAsync);
	flexGetVelocities(m_solver, m_velocities, m_numberOfParticles, eFlexMemoryHostAsync);

	flexSetFence();
	flexWaitFence();

	std::vector<vec3> particlePositions;
	for (int i = 0; i < m_numberOfParticles; ++i)
	{
		float x = m_particles[i * 4 + 0];
		float y = m_particles[i * 4 + 1];
		float z = m_particles[i * 4 + 2];
		particlePositions.push_back(vec3(x, y, z));
	}
	//for (int i = 0; i < g_cloth->mNumParticles; ++i)
	//{
	//	float x = g_cloth->mParticles[i * 4 + 0];
	//	float y = g_cloth->mParticles[i * 4 + 1];
	//	float z = g_cloth->mParticles[i * 4 + 2];
	//	particlePositions.push_back(vec3(x, y, z));
	//}

	//m_renderer->ModifyMesh(m_clothModel, particlePositions);
}


void TestScene::Draw()
{
	m_renderer->Draw();
}

void TestScene::AddCloth(unsigned int a_dimensions)
{
	const int sqrDimensions = a_dimensions * a_dimensions;
	int clothPhase = flexMakePhase(1, eFlexPhaseSelfCollide);
	for (unsigned int r = 0; r < a_dimensions; ++r)
	{
		for (unsigned c = 0; c < a_dimensions; ++c)
		{
			m_particles[(r * a_dimensions + c) * 4 + 0]  = (float)c - a_dimensions / 2;	//x
			m_particles[(r * a_dimensions + c) * 4 + 1]  = 10;								//y
			m_particles[(r * a_dimensions + c) * 4 + 2]  = (float)r - a_dimensions / 2;	//z

			if ((r == 0 && c == 0) || (r == 0 && c == a_dimensions - 1))
				m_particles[(r * a_dimensions + c) * 4 + 3] = 0.0f;						//inverse mass
			else
				m_particles[(r * a_dimensions + c) * 4 + 3] = 1.0f;						//inverse mass

			m_velocities[(r * a_dimensions + c) * 3 + 0] = 0;	//x
			m_velocities[(r * a_dimensions + c) * 3 + 1] = 0;	//y
			m_velocities[(r * a_dimensions + c) * 3 + 2] = 0;	//z

			m_phases[r * a_dimensions + c] = clothPhase;

			m_verticies[(r * a_dimensions + c) * 3 + 0] = (float)c - a_dimensions / 2;
			m_verticies[(r * a_dimensions + c) * 3 + 1] = 10;
			m_verticies[(r * a_dimensions + c) * 3 + 2] = (float)r - a_dimensions / 2;
		}
	}

	flexSetParticles(m_solver, m_particles, sqrDimensions, eFlexMemoryHostAsync);
	flexSetVelocities(m_solver, m_velocities, sqrDimensions, eFlexMemoryHostAsync);
	flexSetPhases(m_solver, m_phases, sqrDimensions, eFlexMemoryHostAsync);

	m_clothModel = m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1);

	const int numberOfTriangles = (a_dimensions - 1) * (a_dimensions - 1) * 2;
	unsigned int index = 0;
	for (unsigned int r = 0; r < a_dimensions - 1; ++r)
	{
		for (unsigned int c = 0; c < a_dimensions - 1; ++c)
		{
			//Triangle 1
			m_indices[index++] = r * a_dimensions + c;
			m_indices[index++] = (r + 1) * a_dimensions + c;
			m_indices[index++] = (r + 1) * a_dimensions + (c + 1);
			
			//Triangle 2
			m_indices[index++] = r * a_dimensions + c;
			m_indices[index++] = (r + 1) * a_dimensions + (c + 1);
			m_indices[index++] = r * a_dimensions + (c + 1);

			////Triangle 1
			//m_indices[index++] = r * a_dimensions + c;
			//m_indices[index++] = (r + 1) * a_dimensions + (c + 1);
			//m_indices[index++] = (r + 1) * a_dimensions;
			//
			////Triangle 2
			//m_indices[index++] = r * a_dimensions + c;
			//m_indices[index++] = r * a_dimensions + (c + 1);
			//m_indices[index++] = (r + 1) * a_dimensions + (c + 1);
		}
	}

	g_cloth = flexExtCreateClothFromMesh(m_particles, sqrDimensions, m_indices, numberOfTriangles, 0.9f, 1.0f, 1.0f, 5.0f, 0.0f);

	for (int i = 0; i < sqrDimensions; ++i)
	{
		m_activeParticles[i] = i;
	}

	flexSetActive(m_solver, m_activeParticles, sqrDimensions, eFlexMemoryHostAsync);
	flexSetSprings(m_solver, g_cloth->mSpringIndices, g_cloth->mSpringRestLengths, g_cloth->mSpringCoefficients, g_cloth->mNumSprings, eFlexMemoryHostAsync);

	flexSetDynamicTriangles(m_solver, m_indices, NULL, numberOfTriangles, eFlexMemoryHostAsync);
	//flexSetTriangles(m_solver, indices, verticies, numberOfTriangles, sqrDimensions, m_particleRadius, FlexMemory::eFlexMemoryHost);


	//m_clothModels.push_back(m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1));
	//glm::quat rot = glm::quat(a_pose.q.w, a_pose.q.x, a_pose.q.y, a_pose.q.z);
	//m_renderer->SetTransform((mat4)rot * glm::translate(vec3(a_pose.p.x, a_pose.p.y, a_pose.p.z)), m_clothModels[m_clothModels.size() - 1]);
}

void TestScene::AddBox(vec3 a_position, glm::quat a_rotation)
{
	unsigned int numberOfVertices, numberOfIndices = -1;
	float* vertices = nullptr;
	int* indices = nullptr;

	unsigned int cube =  m_renderer->LoadOBJ("../data/cube.obj", numberOfVertices, vertices, numberOfIndices, indices);

	FlexExtAsset* g_cube = flexExtCreateRigidFromMesh(vertices, numberOfVertices, indices, numberOfIndices, 1.0f, 0.0f);
	
	int phase = flexMakePhase(2, eFlexPhaseGroupMask);
	int* phases = new int[g_cube->mNumParticles];
	float* velocities = new float[g_cube->mNumParticles * 3];
	for (unsigned int i = 0; i < g_cube->mNumParticles; ++i)
	{
		phases[i] = phase;
		m_activeParticles[i] = i;
	}

	vec3* cubeRestPositions = new vec3[g_cube->mNumShapeIndices];
	CalculateRigidOffsets((vec4*)g_cube->mParticles, g_cube->mShapeOffsets, g_cube->mShapeIndices, g_cube->mNumShapes, cubeRestPositions);

	flexSetParticles(m_solver, g_cube->mParticles, g_cube->mNumParticles, eFlexMemoryHostAsync);
	flexSetVelocities(m_solver, velocities, g_cube->mNumParticles, eFlexMemoryHostAsync);
	flexSetPhases(m_solver, phases, g_cube->mNumParticles, eFlexMemoryHostAsync);
	flexSetActive(m_solver, m_activeParticles, g_cube->mNumParticles, eFlexMemoryHostAsync);
	//This doesn't work. In the flex demo, they occasionally pass NULL into the normals component of this function, however it's the main thing that I suspect of breaking.
	//Redo this with a proper normal calculation, to see if that fixes it.
	flexSetRigids(m_solver, g_cube->mShapeOffsets, g_cube->mShapeIndices, (float*)cubeRestPositions, NULL, g_cube->mShapeCoefficients, &a_rotation[0], &a_position[0], g_cube->mNumShapes, eFlexMemoryHostAsync);
	
	delete[] phases;
	delete[] velocities;
	delete[] vertices;
	delete[] indices;
}


// Copy + pasted from FleX demo code and modified to use glm vectors instead.
void TestScene::CalculateRigidOffsets(const vec4* restPositions, const int* offsets, const int* indices, int numRigids, vec3* localPositions)
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

			localPositions[count++] = vec3(restPositions[r]) - com;
		}
	}
}