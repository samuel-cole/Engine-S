#include "PhysicsGame.h"
#include "Renderer.h"
#include "LevelData.h"
#include "InputManager.h"
#include "TrackerCamera.h"

void PhysicsGame::SetGravity(vec3 a_gravity)
{
	float magnitude = glm::length(a_gravity);
	m_gravityStrength = magnitude;
	m_oldGravityStrength = magnitude;
	vec3 gravityDir = a_gravity / magnitude;
	m_gravityDir = gravityDir;
	m_oldGravityDir = gravityDir;
}

bool PhysicsGame::IsNearlyEqual(float a_float1, float a_float2)
{
	if (abs(a_float1 - a_float2) < 0.01f)
		return true;
	return false;
}
bool PhysicsGame::IsNearlyEqual(vec3 a_vec1, vec3 a_vec2)
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

	LoadLevel(0, true);

	return 0;
}

void PhysicsGame::LoadLevel(const int a_level, const bool a_startingGame)
{
	m_loadedLevel = a_level;

	//If the game is already being played, all of the data from the current game needs to be unloaded.
	if (!a_startingGame)
	{
		m_hazardShapeIndices.clear();
		m_renderer->ResetPointLights();
		DestroyAll();
	}

	//If we are starting the game, this will remove debug lighting variables, that aren't important to the game.
	//If this isn't the start of the game (just loading a new level), this will ensure that only the properties that are needed for that level are displayed.
	//The bar is deleted/recreated instead of just cleared because clearing the bar while it is being modified leads to Anttweakbar becoming unresponsive.
	TwDeleteBar(m_debugBar);
	m_debugBar = TwNewBar("Options Bar");
	float refreshRate = 0.01f;
	TwSetParam(m_debugBar, NULL, "refresh", TwParamValueType::TW_PARAM_FLOAT, 1, &refreshRate);

	TwAddVarRO(m_debugBar, "Level Time", TW_TYPE_FLOAT, &m_currentLevelTime, "");
	TwAddVarRW(m_debugBar, "Time Playing", TW_TYPE_BOOL8, &m_updateFleXScene, "");

	//Initialise the new level.
	m_gravityDir = vec3(0, -1, 0);
	m_gravityStrength = 9.8f;
	m_oldGravityDir = vec3(0, -1, 0);
	m_oldGravityStrength = 9.8f;

	m_restitution = 0.2f;
	m_oldRestitution = 0.2f;

	m_bouyancy = 2.0f;
	m_oldBouyancy = 2.0f;

	m_modifiablePropertiesMask = LevelData::LoadLevel(this, a_level, m_goalObjectIndex, m_targetShapeIndex, m_hazardShapeIndices);

	if (m_modifiablePropertiesMask == -1)
	{	
		//We've finished the game/hit an invalid level, restart the game instead.
		m_loadedLevel = 0;
		m_modifiablePropertiesMask = LevelData::LoadLevel(this, 0, m_goalObjectIndex, m_targetShapeIndex, m_hazardShapeIndices);
	}

	if ((m_modifiablePropertiesMask & (1 << GRAVITY)) > 0)
	{
		TwAddVarRW(m_debugBar, "Gravity", TW_TYPE_DIR3F, &m_gravityDir[0], "");
		TwAddVarRW(m_debugBar, "Gravity Strength", TW_TYPE_FLOAT, &m_gravityStrength, "min=0 max = 30");
	}
	if ((m_modifiablePropertiesMask & (1 << RESTITUTION)) > 0)
		TwAddVarRW(m_debugBar, "Restitution", TW_TYPE_FLOAT, &m_restitution, "min=0 max=1000 step = 0.02");
	if ((m_modifiablePropertiesMask & (1 << BOUYANCY)) > 0)
		TwAddVarRW(m_debugBar, "Fluid Gravity Strength", TW_TYPE_FLOAT, &m_bouyancy, "min=0 max=20 step=0.1");
	if ((m_modifiablePropertiesMask & (1 << PAUSE_GAME)) > 0)
		m_updateFleXScene = false;

	//Set goal object materials.
	if (m_goalObjectIndex != -2)
	{
		//The goal object is a box.
		m_renderer->LoadTexture("../data/colours/blue.png", m_boxModels[m_goalObjectIndex]);
		m_renderer->LoadAmbient("../data/colours/blue.png", m_boxModels[m_goalObjectIndex]);
		m_renderer->LoadSpecularMap("../data/colours/blue.png", m_boxModels[m_goalObjectIndex]);
		m_goalObjectLightIndex = m_renderer->CreatePointLight(vec3(0.0f, 0.0f, 0.2f), 5.0f, false, m_renderer->GetPosition(m_boxModels[m_goalObjectIndex]));
		m_camera->SetObjectToTrack(m_boxModels[m_goalObjectIndex], m_renderer);
	}
	else
	{
		//The goal object is the fluid in the scene.
		for (unsigned int i = 0; i < m_fluidRenderHandles.size(); ++i)
		{
			m_renderer->LoadTexture("../data/colours/blue.png", m_fluidRenderHandles[i]);
			m_renderer->LoadAmbient("../data/colours/blue.png", m_fluidRenderHandles[i]);
			m_renderer->LoadSpecularMap("../data/colours/blue.png", m_fluidRenderHandles[i]);
		}

		m_camera->SetObjectToTrack(m_fluidRenderHandles[0], m_renderer);
	}

	//Set target shape materials.
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
	g_params.mRestitution = m_restitution;
	g_params.mBuoyancy = m_bouyancy;
	flexSetParams(g_solver, &g_params);

	m_currentLevelTime = 0.0f;

	InputManager::SetupAntBarCallbacks();
}

void PhysicsGame::Update(float a_deltaTime)
{
	if ((m_modifiablePropertiesMask & (1 << GRAVITY)) > 0)
	{
		if (!IsNearlyEqual(m_oldGravityDir, m_gravityDir) || !IsNearlyEqual(m_oldGravityStrength, m_gravityStrength))
		{
			m_oldGravityDir = m_gravityDir;
			m_oldGravityStrength = m_gravityStrength;
			g_params.mGravity[0] = m_gravityDir.x * m_gravityStrength;
			g_params.mGravity[1] = m_gravityDir.y * m_gravityStrength;
			g_params.mGravity[2] = m_gravityDir.z * m_gravityStrength;
			flexSetParams(g_solver, &g_params);
		}
	}

	if ((m_modifiablePropertiesMask & (1 << RESTITUTION)) > 0)
	{
		if (!IsNearlyEqual(m_restitution, m_oldRestitution))
		{
			m_oldRestitution = m_restitution;
			g_params.mRestitution = m_restitution;
			flexSetParams(g_solver, &g_params);
		}
	}

	if ((m_modifiablePropertiesMask & (1 << BOUYANCY)) > 0)
	{
		if (!IsNearlyEqual(m_bouyancy, m_oldBouyancy))
		{
			m_oldBouyancy = m_bouyancy;
			g_params.mBuoyancy = m_bouyancy;
			flexSetParams(g_solver, &g_params);
		}
	}

	if (InputManager::GetKeyDown(Keys::SPACE))
	{
		m_updateFleXScene = !m_updateFleXScene;
	}
	if (InputManager::GetKeyDown(Keys::R))
	{
		LoadLevel(m_loadedLevel);
	}

	if (m_updateFleXScene)
		m_currentLevelTime += a_deltaTime;

	FleXBase::Update(a_deltaTime);

	if (m_goalObjectIndex != -2)
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

	int goalObjectStartIndex;
	int goalObjectEndIndex;

	bool fluidGoal = m_goalObjectIndex == -2;
	if (fluidGoal)
	{
		goalObjectStartIndex = 0;//m_fluidParticles[0];
		goalObjectEndIndex = m_fluidParticles.size() - 1; //m_fluidParticles[m_fluidParticles.size() - 1];
	}
	else
	{
		goalObjectStartIndex = m_rigidOffsets[m_goalObjectIndex];
		goalObjectEndIndex = m_rigidOffsets[m_goalObjectIndex + 1];
	}

	for (int i = goalObjectStartIndex; i < goalObjectEndIndex; ++i)
	{
		int contactIndex;
		if (fluidGoal)
			contactIndex = m_fluidParticles[i];
		else
			contactIndex = contactIndices[m_rigidIndices[i]];

		const unsigned char count = contactCounts[contactIndex];
	
		for (int c = 0; c < count; ++c)
		{
			vec4 velocity = contactVelocities[contactIndex * MAX_PARTICLE_CONTACTS + c];
			const int shapeID = (int)velocity.w;
	
			if (shapeID == m_targetShapeIndex)
			{
				printf("You win! \n");
				printf("Level completed in: %f seconds! \n", m_currentLevelTime);
				LoadLevel(m_loadedLevel + 1);
				return;
			}
			else
			{
				for (unsigned int j = 0; j < m_hazardShapeIndices.size(); ++j)
				{
					if (shapeID == m_hazardShapeIndices[j])
					{
						printf("You lose! \n");
						LoadLevel(m_loadedLevel);
						return;
					}
				}
			}
		}
	}
}