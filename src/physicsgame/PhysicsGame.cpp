#include "PhysicsGame.h"
#include "Renderer.h"
#include "LevelData.h"
#include "InputManager.h"

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

int PhysicsGame::Init()
{
	int baseInit = FleXBase::Init();
	if (baseInit != 0)
		return baseInit;

	m_gravityDir = vec3(0, -1, 0);
	m_gravityStrength = 9.8f;
	m_oldGravityDir = vec3(0, -1, 0);
	m_oldGravityStrength = 9.8f;

	TwAddVarRW(m_debugBar, "Gravity", TW_TYPE_DIR3F, &m_gravityDir[0], "");
	TwAddVarRW(m_debugBar, "Gravity Strength", TW_TYPE_FLOAT, &m_gravityStrength, "min=0 max = 30");

	//TODO: load in a level.
	LevelData::LoadLevel(this, 0, m_goalObjectIndex, m_targetShapeIndex);

	//TODO: replace these with better textures.
	m_renderer->LoadTexture("../data/crate.png", m_boxModels[m_goalObjectIndex]);
	m_renderer->LoadAmbient("../data/crate.png", m_boxModels[m_goalObjectIndex]);
	m_renderer->LoadTexture("../data/crate.png", m_shapeModels[m_targetShapeIndex]);
	m_renderer->LoadAmbient("../data/crate.png", m_shapeModels[m_targetShapeIndex]);

	flexGetParams(g_solver, &g_params);
	g_params.mGravity[0] = m_gravityDir.x * m_gravityStrength;
	g_params.mGravity[1] = m_gravityDir.y * m_gravityStrength;
	g_params.mGravity[2] = m_gravityDir.z * m_gravityStrength;
	flexSetParams(g_solver, &g_params);

	return 0;
}

void PhysicsGame::Update(float a_deltaTime)
{
	if ((m_modifiablePropertiesMask & (1 << GRAVITY)) > 0)
	{
		if (!isNearlyEqual(m_oldGravityDir, m_gravityDir) || !isNearlyEqual(m_oldGravityStrength, m_gravityStrength))
		{
			m_oldGravityDir = m_gravityDir;
			m_oldGravityStrength = m_gravityStrength;
			g_params.mGravity[0] = m_gravityDir.x * m_gravityStrength;
			g_params.mGravity[1] = m_gravityDir.y * m_gravityStrength;
			g_params.mGravity[2] = m_gravityDir.z * m_gravityStrength;
			flexSetParams(g_solver, &g_params);
		}
	}

	if (InputManager::GetKey(Keys::SPACE))
	{
		m_updateFleXScene = !m_updateFleXScene;
	}

	FleXBase::Update(a_deltaTime);

	if (CheckWin())
	{
		printf("You win! \n");
	}
}

bool PhysicsGame::CheckWin()
{
	const int MAX_PARTICLE_CONTACTS = 4;

	std::vector<vec4> contactPlanes(m_numberOfParticles * MAX_PARTICLE_CONTACTS);
	std::vector<vec4> contactVelocities(m_numberOfParticles * MAX_PARTICLE_CONTACTS);
	std::vector<int> contactIndices(m_numberOfParticles);
	std::vector<unsigned char> contactCounts(m_numberOfParticles);

	flexGetContacts(g_solver, (float*)&contactPlanes[0], (float*)&contactVelocities[0], &contactIndices[0], &contactCounts[0], eFlexMemoryHost);

	for (int i = m_rigidOffsets[m_goalObjectIndex]; i < m_rigidOffsets[m_goalObjectIndex + 1]; ++i)
	{
		const int contactIndex = contactIndices[i];
		const unsigned char count = contactCounts[contactIndex];
	
		for (int c = 0; c < count; ++c)
		{
			vec4 velocity = contactVelocities[contactIndex * MAX_PARTICLE_CONTACTS + c];
			const int shapeID = (int)velocity.w;
	
			if (shapeID == m_targetShapeIndex)
				return true;
		}
	}
	return false;
}