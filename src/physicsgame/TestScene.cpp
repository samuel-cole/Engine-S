#include "TestScene.h"
#include "WalkCamera.h"
#include "Renderer.h"

#include <flex.h>

int TestScene::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new WalkCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);

	//m_renderer->GenerateGrid(9, 9);

	flexInit();
	m_solver = flexCreateSolver(65536, 0);

	FlexParams params;
	params.mGravity[0] = 0.0f;
	params.mGravity[1] = -9.8f;
	params.mGravity[2] = 0.0f;

	m_particleRadius = params.mRadius;

	flexSetParams(m_solver, &params);

	//float particles[100 * 4];
	//float velocities[100 * 3];
	//int phases[100];
	//int rigidPhase = flexMakePhase(0, 0);
	//for (int i = 0; i < 100; ++i)
	//{
	//	particles[i * 4 + 0] = i / 10;
	//	particles[i * 4 + 1] = 0;
	//	particles[i * 4 + 2] = i % 10;
	//	particles[i * 4 + 3] = 1;
	//
	//	velocities[i * 3 + 0] = 0;
	//	velocities[i * 3 + 1] = 0;
	//	velocities[i * 3 + 2] = 0;
	//
	//	phases[i] = rigidPhase;
	//}
	//
	//flexSetParticles(m_solver, particles, 100, FlexMemory::eFlexMemoryHost);
	//flexSetVelocities(m_solver, velocities, 100, FlexMemory::eFlexMemoryHost);
	//flexSetPhases(m_solver, phases, 100, FlexMemory::eFlexMemoryHost);
	//
	//m_clothModel = m_renderer->GenerateGrid(9, 9);

	AddCloth(10);

	return 0;
}

int TestScene::Deinit()
{
	flexDestroySolver(m_solver);
	flexShutdown();

	m_renderer->CleanupBuffers();
	delete m_renderer;
	delete m_camera;

	return Application::Deinit();
}

void TestScene::Update(float a_deltaTime)
{
	m_camera->Update(a_deltaTime);

	flexUpdateSolver(m_solver, a_deltaTime, 1, nullptr);

	//float* particles = new float[100 * 4];
	//flexGetParticles(m_solver, particles, 100, eFlexMemoryHost);
	//
	//float* velocities = new float[100 * 3];
	//flexGetVelocities(m_solver, velocities, 100, eFlexMemoryHost);
	//
	////Represent cloth objects within the scene.
	//std::vector<vec3> particlePositions;
	//for (unsigned int i = 0; i < 100; ++i)
	//{
	//	float x = particles[i * 4 + 0];
	//	float y = particles[i * 4 + 1];
	//	float z = particles[i * 4 + 2];
	//	particlePositions.push_back(vec3(x, y, z));
	//}
	//m_renderer->ModifyMesh(m_clothModel, particlePositions);
	//
	//delete[] velocities;
	//
	//delete[] particles;
}


void TestScene::Draw()
{
	m_renderer->Draw();
}

void TestScene::AddCloth(unsigned int a_dimensions)
{
	const int sqrDimensions = a_dimensions * a_dimensions;
	float* particles = new float[sqrDimensions * 4];
	float* velocities = new float[sqrDimensions * 3];
	int* phases = new int[sqrDimensions];
	int clothPhase = flexMakePhase(1, eFlexPhaseSelfCollide);
	float* verticies = new float[sqrDimensions * 3];
	for (unsigned int r = 0; r < a_dimensions; ++r)
	{
		for (unsigned c = 0; c < a_dimensions; ++c)
		{
			particles[(r * a_dimensions + c) * 4 + 0]  = (float)c - a_dimensions / 2;	//x
			particles[(r * a_dimensions + c) * 4 + 1]  = 0;								//y
			particles[(r * a_dimensions + c) * 4 + 2]  = (float)r - a_dimensions / 2;	//z
			particles[(r * a_dimensions + c) * 4 + 3]  = 1.0f;							//inverse mass

			velocities[(r * a_dimensions + c) * 3 + 0] = 0;	//x
			velocities[(r * a_dimensions + c) * 3 + 1] = 0;	//y
			velocities[(r * a_dimensions + c) * 3 + 2] = 0;	//z

			phases[r * a_dimensions + c] = clothPhase;

			verticies[(r * a_dimensions + c) * 3 + 0] = (float)c - a_dimensions / 2;
			verticies[(r * a_dimensions + c) * 3 + 1] = 0;
			verticies[(r * a_dimensions + c) * 3 + 2] = (float)r - a_dimensions / 2;
		}
	}

	//The quads field that this primitives variable is used for is essentially the physX equivalent of OpenGL indices (IBOs).
	//PxU32* primitives = new PxU32[(a_dimensions - 1) * (a_dimensions - 1) * 4];
	//unsigned int index = 0;
	//for (unsigned int r = 0; r < (a_dimensions - 1); ++r)
	//{
	//	for (unsigned int c = 0; c < (a_dimensions - 1); ++c)
	//	{
	//		primitives[index++] = r * a_dimensions + c;
	//		primitives[index++] = r * a_dimensions + c + 1;
	//		primitives[index++] = (r + 1) * a_dimensions + c;
	//		primitives[index++] = (r + 1) * a_dimensions + c + 1;
	//	}
	//}
	flexSetParticles(m_solver, particles, sqrDimensions, FlexMemory::eFlexMemoryHost);
	flexSetVelocities(m_solver, velocities, sqrDimensions, FlexMemory::eFlexMemoryHost);
	flexSetPhases(m_solver, phases, sqrDimensions, FlexMemory::eFlexMemoryHost);

	m_clothModel = m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1);

	const int numberOfTriangles = (a_dimensions - 1) * (a_dimensions - 1) * 2;
	unsigned int index = 0;
	int* indices = new int[numberOfTriangles * 3];
	for (int r = 0; r < a_dimensions - 1; ++r)
	{
		for (int c = 0; c < a_dimensions - 1; ++c)
		{
			//Triangle 1
			indices[index++] = r * a_dimensions + c;
			indices[index++] = (r + 1) * a_dimensions + c;
			indices[index++] = (r + 1) * a_dimensions + (c + 1);

			//Triangle 2
			indices[index++] = r * a_dimensions + c;
			indices[index++] = (r + 1) * a_dimensions + (c + 1);
			indices[index++] = r * a_dimensions + (c + 1);
		}
	}

	//TODO: This line is causing a crash when the program is closed.
	flexSetTriangles(m_solver, indices, verticies, numberOfTriangles, sqrDimensions, m_particleRadius, FlexMemory::eFlexMemoryHost);


	//Fill in the description object.
	//PxClothMeshDesc clothDesc;
	//clothDesc.points.data = vertices;
	//clothDesc.points.count = a_dimensions * a_dimensions;
	//clothDesc.points.stride = sizeof(PxClothParticle);
	//clothDesc.invMasses.data = &vertices->invWeight;
	//clothDesc.invMasses.count = a_dimensions * a_dimensions;
	//clothDesc.invMasses.stride = sizeof(PxClothParticle);
	//clothDesc.quads.data = primitives;
	//clothDesc.quads.count = (a_dimensions - 1) * (a_dimensions - 1);
	//clothDesc.quads.stride = sizeof(PxU32) * 4;

	//Create the cloth.
	//PxClothFabric* fabric = PxClothFabricCreate(*g_physics, clothDesc, PxVec3(0, -1, 0));
	//PxCloth* cloth = g_physics->createCloth(a_pose, *fabric, vertices, PxClothFlags());
	//cloth->setClothFlag(PxClothFlag::eSCENE_COLLISION, true);
	//cloth->setSolverFrequency(240.0f);
	//g_physicsScene->addActor(*cloth);
	//g_physicsCloths.push_back(cloth);

	delete[] particles;
	delete[] velocities;
	delete[] phases;
	delete[] verticies;
	delete[] indices;

	//m_clothModels.push_back(m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1));
	//glm::quat rot = glm::quat(a_pose.q.w, a_pose.q.x, a_pose.q.y, a_pose.q.z);
	//m_renderer->SetTransform((mat4)rot * glm::translate(vec3(a_pose.p.x, a_pose.p.y, a_pose.p.z)), m_clothModels[m_clothModels.size() - 1]);
}
