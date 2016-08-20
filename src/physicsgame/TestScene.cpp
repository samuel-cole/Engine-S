#include "TestScene.h"
#include "WalkCamera.h"
#include "Renderer.h"

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

	//m_renderer->GenerateGrid(9, 9);

	flexInit();
	m_solver = flexCreateSolver(65536, 0);

	FlexParams params;
	params.mGravity[0] = 0.0f;
	params.mGravity[1] = -9.8f;
	params.mGravity[2] = 0.0f;

	m_particleRadius = params.mRadius;

	params.mNumIterations = 2;

	flexSetParams(m_solver, &params);

	AddCloth(10);

	int version = flexGetVersion();

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

	flexUpdateSolver(m_solver, 1.0f/60.0f, 1, NULL);

	std::vector<vec3> particlePositions;
	for (int i = 0; i < g_cloth->mNumParticles; ++i)
	{
		float x = g_cloth->mParticles[i * 4 + 0];
		float y = g_cloth->mParticles[i * 4 + 1];
		float z = g_cloth->mParticles[i * 4 + 2];
		particlePositions.push_back(vec3(x, y, z));
	}
	m_renderer->ModifyMesh(m_clothModel, particlePositions);

	//Alternate method for drawing, uses the particle array instead of the object's one.
	//std::vector<vec3> particlePositions;
	//int numberOfParticles = g_cloth->mNumParticles;
	//float* particles = new float[4 * numberOfParticles];
	//flexGetParticles(m_solver, particles, numberOfParticles, FlexMemory::eFlexMemoryHost);
	//for (int i = 0; i < numberOfParticles; ++i)
	//{
	//	float x = particles[i * 4 + 0];
	//	float y = particles[i * 4 + 1];
	//	float z = particles[i * 4 + 2];
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

			if ((r == 0 && c == 0) || (r == 0 && c == a_dimensions - 1))
				particles[(r * a_dimensions + c) * 4 + 3] = 0.0f;						//inverse mass
			else
				particles[(r * a_dimensions + c) * 4 + 3] = 1.0f;						//inverse mass

			velocities[(r * a_dimensions + c) * 3 + 0] = 0;	//x
			velocities[(r * a_dimensions + c) * 3 + 1] = 0;	//y
			velocities[(r * a_dimensions + c) * 3 + 2] = 0;	//z

			phases[r * a_dimensions + c] = clothPhase;

			verticies[(r * a_dimensions + c) * 3 + 0] = (float)c - a_dimensions / 2;
			verticies[(r * a_dimensions + c) * 3 + 1] = 0;
			verticies[(r * a_dimensions + c) * 3 + 2] = (float)r - a_dimensions / 2;
		}
	}

	flexSetParticles(m_solver, particles, sqrDimensions, FlexMemory::eFlexMemoryHost);
	flexSetVelocities(m_solver, velocities, sqrDimensions, FlexMemory::eFlexMemoryHost);
	flexSetPhases(m_solver, phases, sqrDimensions, FlexMemory::eFlexMemoryHost);

	m_clothModel = m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1);

	const int numberOfTriangles = (a_dimensions - 1) * (a_dimensions - 1) * 2;
	unsigned int index = 0;
	int* indices = new int[numberOfTriangles * 3];
	for (unsigned int r = 0; r < a_dimensions - 1; ++r)
	{
		for (unsigned int c = 0; c < a_dimensions - 1; ++c)
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

	g_cloth = flexExtCreateClothFromMesh(particles, sqrDimensions, indices, numberOfTriangles, 0.5f, 0.5f, 0.9f, 0.1f, 0.1f);

	int* activeParticles = new int[sqrDimensions];
	for (int i = 0; i < sqrDimensions; ++i)
	{
		activeParticles[i] = i;
	}

	flexSetActive(m_solver, activeParticles, sqrDimensions, FlexMemory::eFlexMemoryHost);

	//TODO: This line is causing a crash when the program is closed.
	flexSetDynamicTriangles(m_solver, indices, NULL, numberOfTriangles, FlexMemory::eFlexMemoryHost);
	//flexSetTriangles(m_solver, indices, verticies, numberOfTriangles, sqrDimensions, m_particleRadius, FlexMemory::eFlexMemoryHost);

	delete[] activeParticles;
	delete[] particles;
	delete[] velocities;
	delete[] phases;
	delete[] verticies;
	delete[] indices;

	//m_clothModels.push_back(m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1));
	//glm::quat rot = glm::quat(a_pose.q.w, a_pose.q.x, a_pose.q.y, a_pose.q.z);
	//m_renderer->SetTransform((mat4)rot * glm::translate(vec3(a_pose.p.x, a_pose.p.y, a_pose.p.z)), m_clothModels[m_clothModels.size() - 1]);
}

void TestScene::AddBox()
{

}
