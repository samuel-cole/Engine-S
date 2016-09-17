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

	AddCloth(10);
	AddBox(vec3(10, 15, 0), quat(vec3(30, 25, 70)));
	AddBox(vec3(5, 15, 0), quat(vec3(30, 25, 70)));
	AddBox(vec3(0, 15, 5), quat(vec3(30, 25, 70)));

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
		flexGetParams(m_solver, &params);
		params.mGravity[0] = m_gravityDir.x * m_gravityStrength;
		params.mGravity[1] = m_gravityDir.y * m_gravityStrength;
		params.mGravity[2] = m_gravityDir.z * m_gravityStrength;
		flexSetParams(m_solver, &params);
	}

	FleXBase::Update(a_deltaTime);

	DebugUpdate(a_deltaTime);
}

void TestScene::DebugUpdate(float a_deltaTime)
{
	if (InputManager::GetKey(Keys::ENTER))
		AddBox(vec3(0, 15, 0), quat(vec3(30, 25, 70)));
	
	if (InputManager::GetKey(Keys::SPACE))
	{
		for (unsigned int i = 0; i < g_cubes.size(); ++i)
		{
			printf("Object %i after update: %f, %f, %f \n", i, m_positions[i].x, m_positions[i].y, m_positions[i].z);
		}
	}
}