#include "TestScene.h"
#include "InputManager.h"

#include <flex.h>

bool isNearlyEqual(float a_float1, float a_float2)
{
	if (abs(a_float1 - a_float2) < 0.01f)
		return true;
	return false;
}
bool isNearlyEqual(vec3 a_vec1, vec3 a_vec2)
{
	if (glm::length2(a_vec1 - a_vec2) < 0.01f)
		return true;
	return false;
}

int TestScene::Init()
{
	int baseInit = FleXBase::Init();
	if (baseInit != 0)
		return baseInit;

	m_gravityDir = vec3(0, -1, 0);
	m_gravityStrength = 9.8f;
	m_oldGravityDir = vec3(0, -1, 0);
	m_oldGravityStrength = 9.8f;

	TwAddVarRW(m_debugBar, "Gravity", TW_TYPE_DIR3F, &m_gravityDir[0], "");
	TwAddVarRW(m_debugBar, "Gravity Strength", TW_TYPE_FLOAT, &m_gravityStrength, "min=0 max=30");

	unsigned int tetherPoints[2] = { 0, 19 };
	AddCloth(20, 2, tetherPoints, 10.0f);
	
	for (int i = -1; i < 2; ++i)
	{
		for (int j = -1; j < 2; ++j)
		{
			AddBox(vec3(i * 5.0f, 20.0f, j * 5.0f), quat(vec3(30, 25, 70)));
		}
	}
	
	tetherPoints[1] = 380;
	AddCloth(20, 2, tetherPoints, 15.0f);

	//AddBox(vec3(0.0f, 20.0f, 0.0f), quat(vec3(0, 0, 0)));

	return 0;
}

void TestScene::Update(float a_deltaTime)
{
	//Gravity Management
	if (!isNearlyEqual(m_oldGravityDir, m_gravityDir) || !isNearlyEqual(m_oldGravityStrength, m_gravityStrength))
	{
		m_oldGravityDir = m_gravityDir;
		m_oldGravityStrength = m_gravityStrength;
		FlexParams params;
		flexGetParams(g_solver, &params);
		params.mGravity[0] = m_gravityDir.x * m_gravityStrength;
		params.mGravity[1] = m_gravityDir.y * m_gravityStrength;
		params.mGravity[2] = m_gravityDir.z * m_gravityStrength;
		flexSetParams(g_solver, &params);
	}

	FleXBase::Update(a_deltaTime);

	DebugUpdate(a_deltaTime);
}

void TestScene::DebugUpdate(float a_deltaTime)
{
	if (InputManager::GetKey(Keys::ENTER))
		//AddBox(vec3(0, 15, 0), quat(vec3(30, 25, 70)));
		AddCloth(6, 0, nullptr, 30.0f);
	
	if (InputManager::GetKey(Keys::SPACE))
	{
		for (unsigned int i = 0; i < g_cubes.size(); ++i)
		{
			printf("Object %i after update: %f, %f, %f \n", i, m_rigidPositions[i].x, m_rigidPositions[i].y, m_rigidPositions[i].z);
		}
	}

	const int MAX_PARTICLE_CONTACTS = 4;
	
	std::vector<vec4> contactPlanes(m_numberOfParticles * MAX_PARTICLE_CONTACTS);
	std::vector<vec4> contactVelocities(m_numberOfParticles * MAX_PARTICLE_CONTACTS);
	std::vector<int> contactIndices(m_numberOfParticles);
	std::vector<unsigned char> contactCounts(m_numberOfParticles);
	
	flexGetContacts(g_solver, (float*)&contactPlanes[0], (float*)&contactVelocities[0], &contactIndices[0], &contactCounts[0], eFlexMemoryHost);
	
	//for (int i = 0; i < m_numberOfActiveParticles; ++i)
	//{
	//	const int contactIndex = contactIndices[i];
	//	const unsigned char count = contactCounts[contactIndex];
	//
	//	for (int c = 0; c < count; ++c)
	//	{
	//		vec4 velocity = contactVelocities[contactIndex * MAX_PARTICLE_CONTACTS + c];
	//
	//		vec3 contactPlane = (vec3)contactPlanes[contactIndex * MAX_PARTICLE_CONTACTS + c];
	//
	//		(vec3&)m_velocities[i] = contactPlane * 1000.0f;
	//	}
	//}
}