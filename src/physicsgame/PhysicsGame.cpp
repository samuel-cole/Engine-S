#include "PhysicsGame.h"
#include "Renderer.h"
#include "LevelData.h"
#include "InputManager.h"
#include "TrackerCamera.h"

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

	LevelData::LoadLevel(this, 0, m_goalObjectIndex, m_targetShapeIndex, m_hazardShapeIndices);

	m_renderer->LoadTexture("../data/colours/blue.png", m_boxModels[m_goalObjectIndex]);
	m_renderer->LoadAmbient("../data/colours/blue.png", m_boxModels[m_goalObjectIndex]);
	m_renderer->LoadSpecularMap("../data/colours/blue.png", m_boxModels[m_goalObjectIndex]);
	m_goalObjectLightIndex = m_renderer->CreatePointLight(vec3(0.0f, 0.0f, 0.2f), 5.0f, false, m_renderer->GetPosition(m_boxModels[m_goalObjectIndex]));
	m_renderer->LoadTexture("../data/colours/green.png", m_shapeModels[m_targetShapeIndex]);
	m_renderer->LoadAmbient("../data/colours/green.png", m_shapeModels[m_targetShapeIndex]);
	m_renderer->LoadSpecularMap("../data/colours/green.png", m_shapeModels[m_targetShapeIndex]);
	m_renderer->CreatePointLight(vec3(0.0f, 0.2f, 0.0f), 3.0f, false, m_renderer->GetPosition(m_shapeModels[m_targetShapeIndex]));

	for (unsigned int i = 0; i < m_hazardShapeIndices.size(); ++i)
	{
		m_renderer->LoadTexture("../data/colours/red.png", m_shapeModels[m_hazardShapeIndices[i]]);
		m_renderer->LoadAmbient("../data/colours/red.png", m_shapeModels[m_hazardShapeIndices[i]]);
		m_renderer->LoadSpecularMap("../data/colours/red.png", m_shapeModels[m_hazardShapeIndices[i]]);
		m_renderer->CreatePointLight(vec3(0.2f, 0.0f, 0.0f), 3.0f, false, m_renderer->GetPosition(m_shapeModels[m_hazardShapeIndices[i]]));
	}

	flexGetParams(g_solver, &g_params);
	g_params.mGravity[0] = m_gravityDir.x * m_gravityStrength;
	g_params.mGravity[1] = m_gravityDir.y * m_gravityStrength;
	g_params.mGravity[2] = m_gravityDir.z * m_gravityStrength;
	flexSetParams(g_solver, &g_params);

	m_camera->SetObjectToTrack(m_boxModels[m_goalObjectIndex], m_renderer);

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

	if (InputManager::GetKeyDown(Keys::SPACE))
	{
		m_updateFleXScene = !m_updateFleXScene;
	}

	FleXBase::Update(a_deltaTime);

	m_renderer->SetLightPosition(m_goalObjectLightIndex, m_rigidPositions[m_goalObjectIndex]);

	CheckWin();
}

void PhysicsGame::CheckWin()
{
	const int MAX_PARTICLE_CONTACTS = 4;

	std::vector<vec4> contactPlanes(m_numberOfParticles * MAX_PARTICLE_CONTACTS);
	std::vector<vec4> contactVelocities(m_numberOfParticles * MAX_PARTICLE_CONTACTS);
	std::vector<int> contactIndices(m_numberOfParticles);
	std::vector<unsigned char> contactCounts(m_numberOfParticles);

	flexGetContacts(g_solver, (float*)&contactPlanes[0], (float*)&contactVelocities[0], &contactIndices[0], &contactCounts[0], eFlexMemoryHost);

	for (int i = m_rigidOffsets[m_goalObjectIndex]; i < m_rigidOffsets[m_goalObjectIndex + 1]; ++i)
	{
		const int contactIndex = contactIndices[m_rigidIndices[i]];
		const unsigned char count = contactCounts[contactIndex];
	
		for (int c = 0; c < count; ++c)
		{
			vec4 velocity = contactVelocities[contactIndex * MAX_PARTICLE_CONTACTS + c];
			const int shapeID = (int)velocity.w;
	
			if (shapeID == m_targetShapeIndex)
				printf("You win! \n");
			else
			{
				for (unsigned int j = 0; j < m_hazardShapeIndices.size(); ++j)
				{
					if (shapeID == m_hazardShapeIndices[j])
					{
						printf("You lose! \n");
					}
				}
			}
		}
	}
}