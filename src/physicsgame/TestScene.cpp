#include "TestScene.h"
#include "WalkCamera.h"
#include "Renderer.h"
#include "glm\gtx\euler_angles.hpp"

#include <flex.h>
#include <flexExt.h>

int TestScene::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new WalkCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);

	m_gravityDir = vec3(0, -1, 0);
	m_gravityStrength = 9.8f;
	m_oldgravityDir = vec3(0, -1, 0);
	m_oldGravityStrength = 9.8f;

	TwAddVarRW(m_debugBar, "Gravity", TW_TYPE_DIR3F, &m_gravityDir[0], "");
	TwAddVarRW(m_debugBar, "Gravity Strength", TW_TYPE_FLOAT, &m_gravityStrength, " min=0 max=30");

	//m_renderer->GenerateGrid(9, 9);

	flexInit();
	m_solver = flexCreateSolver(65536, 0);

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
	(vec4&)params.mPlanes[1] = vec4(0.0f, 0.0f, 1.0f,  10);
	(vec4&)params.mPlanes[2] = vec4(1.0f, 0.0f, 0.0f,  10);
	(vec4&)params.mPlanes[3] = vec4(-1.0f, 0.0f, 0.0f, 10);
	(vec4&)params.mPlanes[4] = vec4(0.0f, 0.0f, -1.0f, 10);

	unsigned int* planes = new unsigned int[5];
	for (int i = 0; i < 5; ++i)
	{
		planes[i] = m_renderer->GenerateGrid(100, 100);
		m_renderer->LoadAmbient("../data/tablecloth.jpg", planes[i]);
		m_renderer->LoadTexture("../data/tablecloth.jpg", planes[i]);
		m_renderer->LoadSpecularMap("../data/tablecloth.jpg", planes[i]);
	}
	mat4 testMat = glm::lookAt(vec3(0, 0, 10), vec3(0, 0, 9), vec3(0, 1, 0));
	m_renderer->SetTransform(glm::lookAt(vec3(0, 0, 10), vec3(0, 0, 11), vec3(0, 1, 0)), planes[1]);
	



	//======================================

	m_particleRadius = params.mRadius;

	//m_particles = (float*)flexAlloc(100 * 4);
	//m_velocities = (float*)flexAlloc(100 * 3);
	//m_phases = (int*)flexAlloc(100);
	//
	//m_verticies = (float*)flexAlloc(100 * 3);
	//m_indices = (int*)flexAlloc(81 * 2 * 3);
	//m_activeParticles = (int*)flexAlloc(100);

	m_particles = new float[100 * 4];
	m_velocities = new float[100 * 3];
	m_phases = new int[100];

	m_verticies = new float[100 * 3];
	m_indices = new int[81 * 2 * 3];
	m_activeParticles = new int[100];

	flexSetParams(m_solver, &params);

	AddCloth(10);

	int version = flexGetVersion();

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

	flexGetParticles(m_solver, m_particles, 100, eFlexMemoryHostAsync);
	flexGetVelocities(m_solver, m_velocities, 100, eFlexMemoryHostAsync);

	int numberOfSprings = g_cloth->mNumSprings;
	int* springIndices = new int[numberOfSprings * 2];
	float* restLengths = new float[numberOfSprings];
	float* coefficients = new float[numberOfSprings];

	springIndices = g_cloth->mSpringIndices;
	restLengths = g_cloth->mSpringRestLengths;
	coefficients = g_cloth->mSpringCoefficients;

	flexSetFence();
	flexWaitFence();

	std::vector<vec3> particlePositions;
	for (int i = 0; i < 100; ++i)
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
	m_renderer->ModifyMesh(m_clothModel, particlePositions);
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

	g_cloth = flexExtCreateClothFromMesh(m_particles, sqrDimensions, m_indices, numberOfTriangles, 0.9f, 1.0f, 100.0f, 5.0f, 0.0f);

	for (int i = 0; i < sqrDimensions; ++i)
	{
		m_activeParticles[i] = i;
	}

	flexSetActive(m_solver, m_activeParticles, sqrDimensions, eFlexMemoryHostAsync);

	flexSetDynamicTriangles(m_solver, m_indices, NULL, numberOfTriangles, eFlexMemoryHostAsync);
	//flexSetTriangles(m_solver, indices, verticies, numberOfTriangles, sqrDimensions, m_particleRadius, FlexMemory::eFlexMemoryHost);


	//m_clothModels.push_back(m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1));
	//glm::quat rot = glm::quat(a_pose.q.w, a_pose.q.x, a_pose.q.y, a_pose.q.z);
	//m_renderer->SetTransform((mat4)rot * glm::translate(vec3(a_pose.p.x, a_pose.p.y, a_pose.p.z)), m_clothModels[m_clothModels.size() - 1]);
}

void TestScene::AddBox()
{

}
