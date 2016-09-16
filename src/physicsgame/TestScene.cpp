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

	m_numberOfParticles = 10000;
	m_numberOfActiveParticles = 0;
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

	//Shape offsets should always start with just a 0 in it.
	m_shapeOffsets.push_back(0);

	flexSetParams(m_solver, &params);

	//AddCloth(sqrtNumberOfParticles);
	AddBox(vec3(0, 10, 0), glm::quat(vec3(30, 25, 70)));
	AddBox(vec3(5, 10, 0), glm::quat(vec3(0, 0, 0)));
	//AddBox(vec3(0, 10, 5), glm::quat(vec3(30, 25, 70)));
	//AddBox(vec3(5, 10, 5), glm::quat(vec3(30, 75, 145)));

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

	if (m_oldgravityDir != m_gravityDir || m_oldGravityStrength != m_gravityStrength)	//TODO: rewrite this to account for float equality issues.
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

	if (InputManager::GetKey(Keys::ENTER))
	{
		AddBox(vec3(0, 10, 0), glm::quat(vec3(30, 25, 70)));
	}

	//flexSetParticles(m_solver, m_particles, 100, eFlexMemoryHostAsync);
	//flexSetVelocities(m_solver, m_velocities, 100, eFlexMemoryHostAsync);

	//printf("Before update: %f, %f, %f \n", m_positions[0].x, m_positions[0].y, m_positions[0].z);
	//flexGetRigidTransforms(m_solver, (float*)&m_rotations[0], (float*)&m_positions[0], eFlexMemoryHostAsync);
	//printf("After update: %f, %f, %f \n", m_positions[0].x, m_positions[0].y, m_positions[0].z);

	if (InputManager::GetKey(Keys::SPACE))
		flexUpdateSolver(m_solver, 1.0f/60.0f, 1, NULL);

	flexGetParticles(m_solver, m_particles, m_numberOfParticles, eFlexMemoryHostAsync);
	flexGetVelocities(m_solver, m_velocities, m_numberOfParticles, eFlexMemoryHostAsync);

	
	flexGetRigidTransforms(m_solver, (float*)&m_rotations[0], (float*)&m_positions[0], eFlexMemoryHostAsync);
	if (InputManager::GetKey(Keys::SPACE))
	{
		for (int i = 0; i < g_cubes.size(); ++i)
		{
			printf("Object %i after update: %f, %f, %f \n", i, m_positions[i].x, m_positions[i].y, m_positions[i].z);
		}
	}


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

	//m_renderer->ModifyMesh(m_boxModel, particlePositions);
	for (unsigned int i = 0; i < g_cubes.size(); ++i)
	{
		m_renderer->SetPosition(m_positions[i], m_boxModels[i]);
		m_renderer->SetRotation(m_rotations[i], m_boxModels[i]);
	}
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
	//flexSetTriangles(m_solver, indices, verticies, numberOfTriangles, sqrDimensions, m_particleRadius, FlexMemory::eFlexMemoryHostAsync);


	//m_clothModels.push_back(m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1));
	//glm::quat rot = glm::quat(a_pose.q.w, a_pose.q.x, a_pose.q.y, a_pose.q.z);
	//m_renderer->SetTransform((mat4)rot * glm::translate(vec3(a_pose.p.x, a_pose.p.y, a_pose.p.z)), m_clothModels[m_clothModels.size() - 1]);
}

void TestScene::AddBox(vec3 a_position, glm::quat a_rotation)
{
	unsigned int numberOfVertices, numberOfIndices = -1;
	float* vertices = nullptr;
	int* indices = nullptr;

	m_positions.push_back(a_position);
	m_rotations.push_back(a_rotation);

	unsigned int model = m_renderer->LoadOBJ("../data/cube.obj", numberOfVertices, vertices, numberOfIndices, indices);
	m_boxModels.push_back(model);
	m_renderer->SetScale(vec3(0.5f, 0.5f, 0.5f), model);
	m_renderer->SetPosition(a_position, model);
	m_renderer->SetRotation(a_rotation, model);

	FlexExtAsset* g_cube = flexExtCreateRigidFromMesh(vertices, numberOfVertices, indices, numberOfIndices, 1.0f, 0.0f);
	
	m_numberOfActiveParticles += g_cube->mNumParticles;

	mat4 transform = m_renderer->GetTransform(model);
	int phase = flexMakePhase(2 + g_cubes.size(), 0);
	for (unsigned int i = g_cube->mNumParticles * g_cubes.size(); i < g_cube->mNumParticles * (g_cubes.size() + 1); ++i)
	{
		int indexInCurrentCube = i - g_cube->mNumParticles * g_cubes.size();
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
	}

	m_shapeOffsets.push_back(m_shapeOffsets[m_shapeOffsets.size() - 1] + g_cube->mShapeOffsets[0]);

	for (int i = 0; i < m_shapeOffsets[m_shapeOffsets.size() - 1] - m_shapeOffsets[m_shapeOffsets.size() - 2]; ++i)
	{
		//Continuing the indices on from the previous shape prevents the shape from flying out of the scene, however it instead attaches all of the shapes to each other in a glitchy-looking way.
		m_shapeIndices.push_back(g_cube->mShapeIndices[i] /*+ m_shapeOffsets[m_shapeOffsets.size() - 2]*/);
	}

	vec3* cubeLocalRestPositions = new vec3[m_shapeIndices.size()];
	vec4* cubeLocalNormals = new vec4[m_shapeIndices.size()];

	CalculateRigidOffsets((vec4*)&m_restPositions[0], &m_shapeOffsets[0], &m_shapeIndices[0], g_cubes.size() + 1, cubeLocalRestPositions, cubeLocalNormals);

	flexSetParticles(m_solver, m_particles, m_numberOfParticles, eFlexMemoryHostAsync);
	flexSetVelocities(m_solver, m_velocities, m_numberOfParticles, eFlexMemoryHostAsync);
	flexSetPhases(m_solver, m_phases, m_numberOfParticles, eFlexMemoryHostAsync);
	flexSetActive(m_solver, m_activeParticles, m_numberOfActiveParticles, eFlexMemoryHostAsync);

	m_shapeCoefficients.push_back(g_cube->mShapeCoefficients[0]);

	//The normals are null here because they seem to be null in some situations in the demo, and that works.
	//In addition, the normals that I'm currently generating will only work for cubes.
	//In addition, it doesn't seem to behave any differently with normals instead of NULL- maybe it's autogenerating them if they aren't passed in?
	flexSetRigids(m_solver, &m_shapeOffsets[0], &m_shapeIndices[0], (float*)cubeLocalRestPositions, /*(float*)cubeLocalNormals*/NULL, &m_shapeCoefficients[0], (float*)&m_rotations[0], (float*)&m_positions[0], g_cubes.size() + 1, eFlexMemoryHostAsync);
	
	delete[] cubeLocalRestPositions;
	delete[] vertices;
	delete[] indices;

	g_cubes.push_back(g_cube);
}


// Copy + pasted from FleX demo code and modified to use glm vectors and return normals.
//This function may not be needed, its end result is just the same as the starting position, as the particles are still at world 0 when this is called.
void TestScene::CalculateRigidOffsets(const vec4* restPositions, const int* offsets, const int* indices, int numRigids, vec3* localPositions, vec4* normals)
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

			normals[count] = vec4(vec3(restPositions[r]) - com, -0.01f);
			localPositions[count++] = vec3(restPositions[r]) - com;
		}
	}
}