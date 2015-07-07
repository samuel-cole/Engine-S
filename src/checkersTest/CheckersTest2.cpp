#include "CheckersTest2.h"
#include "Renderer.h"
#include "WalkCamera.h"
#include "InputManager.h"
#include "CheckersMover.h"
//Used for addlight function
#include "tut13\Tutorial13.h"

#include <iostream>

void TW_CALL BoardGenerate2(void* a_clientData)
{
	CheckersTest2* checkers = (CheckersTest2*)a_clientData;

	Renderer* renderer = checkers->GetRenderer();

	float maxHeight;

	checkers->GetProceduralPhysics()->release();
	renderer->DestroyObject(checkers->GetProceduralPlane());
	unsigned int object;
	checkers->SetProceduralPhysics(checkers->GenerateProceduralPlane(99, 100, 1, vec3(0, 0, 0), checkers->GetPhysicsMaterial(), object, maxHeight, checkers->GetAmplitude(), checkers->GetSeed(), 6, checkers->GetPersistence()));
	checkers->SetProceduralPlane(object);
	renderer->LoadTexture("../data/checkerboard.png", checkers->GetProceduralPlane());
	renderer->LoadAmbient("../data/checkerboard.png", checkers->GetProceduralPlane());

	PxController* player = checkers->GetPlayer();
	PxExtendedVec3 pos = player->getPosition();
	if (pos.x > -51 && pos.x < 51 && pos.z > -51 && pos.z < 51 && pos.y <= maxHeight)
		player->setPosition(PxExtendedVec3(pos.x, maxHeight + 10.0f, pos.z));
}

class PlayerCollisions : public PxUserControllerHitReport
{
	virtual void onShapeHit(const PxControllerShapeHit& a_hit)
	{
		if (a_hit.actor->isRigidDynamic())
		{
			((PxRigidDynamic*)a_hit.actor)->addForce(a_hit.dir * 100);
			((PxRigidDynamic*)a_hit.actor)->setAngularVelocity(-3.0f * a_hit.dir.cross(PxVec3(0, 1, 0)));
		}	
	}
	virtual void onControllerHit(const PxControllersHit& a_hit)		{}
	virtual void onObstacleHit(const PxControllerObstacleHit& a_hit){}
};

int CheckersTest2::Init()
{
	int baseInit = PhysicsBase::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera->SetLookAt(vec3(0, 10, 0), vec3(1, 0, 0), vec3(0, 1, 0));

	g_physicsMaterial = g_physics->createMaterial(0.9f, 0.9f, 0.2f);

	m_gun = m_renderer->LoadOBJ("../data/gun/crossbow.obj");
	m_renderer->LoadTexture("../data/gun/texture.jpg", m_gun);
	m_renderer->LoadAmbient("../data/gun/texture.jpg", m_gun);
	m_renderer->LoadSpecularMap("../data/gun/specular.jpg", m_gun);


	std::vector<std::string> textures;
	std::vector<std::string> normalMaps;
	std::vector<std::string> specularMaps;

	textures.push_back("../data/Enemyelite/EnemyElite3_D.tga");
	textures.push_back("../data/Enemyelite/Alienrifle_D.png");
	normalMaps.push_back("../data/Enemyelite/EnemyElite_N.tga");
	normalMaps.push_back("../data/Enemyelite/Alienrifle_N.png");
	specularMaps.push_back("../data/Enemyelite/EnemyElite_S.tga");
	specularMaps.push_back("../data/Enemyelite/Alienrifle_S.tga");

	m_animatedModel = m_renderer->LoadFBX("../data/Enemyelite/EnemyElite.fbx", &textures, &normalMaps, &specularMaps);
	m_renderer->SetTransform(glm::rotate(glm::translate(m_renderer->GetTransform(m_animatedModel), vec3(-8000, 0, 0)), PxHalfPi, vec3(0, 1, 0)), m_animatedModel);
	m_renderer->LoadAmbient("../data/Enemyelite/EnemyElite3_D.tga", m_animatedModel);

	//Move the gun out of the way.
	m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_animatedModel), vec3(0, -2000000, 0)), m_animatedModel + 1);

	AddBox(g_physicsMaterial, 100.0f, vec3(1, 1, 1), vec3(-10, 100, -10), true);
	m_renderer->LoadTexture("../data/vanquish/upper_d.tga", m_models[m_models.size() - 1]);
	m_renderer->LoadAmbient("../data/vanquish/upper_d.tga", m_models[m_models.size() - 1]);
	m_mover = new CheckersMover(1000.0f, 0.1f, 1.0f, 0.1f, 1000000.0f, (PxRigidDynamic*)(g_physicsActors[g_physicsActors.size() - 1]));

	m_spawnTimer = 0.0f;
	m_walkSpeed = 1.0f;
	m_verticleSpeed = -10.0f;
	m_shootTimer = 0.0f;
	m_shootForce = 100.0f;
	m_animationTimer = 0.0f;

	m_amplitude = 5.0f;
	m_seed = 0;
	m_persistence = 0.3f;

	float buffer;
	g_proceduralPhysics = AddProceduralPlane(99, 100, 1.0f, vec3(0, 0, 0), g_physicsMaterial, m_proceduralPlane, buffer, m_amplitude, 0, 6, m_persistence);
	m_renderer->LoadTexture("../data/checkerboard.png", m_proceduralPlane);
	m_renderer->LoadAmbient("../data/checkerboard.png", m_proceduralPlane);

	TwAddVarRW(m_debugBar, "Shoot Force", TW_TYPE_FLOAT, &m_shootForce, "");
	TwAddVarRW(m_debugBar, "Walk Speed", TW_TYPE_FLOAT, &m_walkSpeed, "");

	TwAddSeparator(m_debugBar, "Lights", "");
	TwAddButton(m_debugBar, "AddLight", AddLight, (void*)(m_renderer), "");

	//Set up the player
	PxControllerManager* controllerManager = PxCreateControllerManager(*g_physicsScene);
	PxCapsuleControllerDesc desc;
	desc.contactOffset = 0.05f;
	desc.height = 3.0f;
	desc.material = g_physicsMaterial;
	desc.nonWalkableMode = PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
	desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
	desc.position = PxExtendedVec3(0, 1000, 0);
	desc.radius = 2.0f; 
	desc.stepOffset = 0.1f;
	g_playerCollisions = new PlayerCollisions();
	desc.reportCallback = g_playerCollisions;
	g_playerController = controllerManager->createController(desc);
	
	//Add a plane
	PxTransform pose = PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(PxHalfPi * 1.0f, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidStatic* plane = PxCreateStatic(*g_physics, pose, PxPlaneGeometry(), *g_physicsMaterial);
	g_physicsScene->addActor(*plane);

	TwAddSeparator(m_debugBar, "Plane", "");
	TwAddVarRW(m_debugBar, "Amplitude", TW_TYPE_FLOAT, &m_amplitude, "");
	TwAddVarRW(m_debugBar, "Persistence", TW_TYPE_FLOAT, &m_persistence, "step=0.1");
	TwAddVarRW(m_debugBar, "Seed", TW_TYPE_UINT32, &m_seed, "");
	TwAddButton(m_debugBar, "Re-generate", BoardGenerate2, (void*)this, "");
	TwAddVarRW(m_debugBar, "AI Difficulty", TW_TYPE_UINT32, &m_aiDifficulty, "");

#pragma region Setup Checkers
	for (unsigned int i = 0; i < 24; ++i)
	{
		unsigned int emitter = m_renderer->CreateEmitter(1000,			  //Max particles
			0.2f,														  //Lifespan minimum 
			2.0f,														  //Lifespan maximum
			0.05f,														  //Velocity minimum
			5.0f,														  //Velocity maximum
			1.0f,														  //Start size
			0.1f,														  //End size
			((i < 12) ? vec4(1, 0, 0, 1) : vec4(0, 0, 1, 1)),			  //Start colour
			((i < 12) ? vec4(1, 1, 0, 1) : vec4(0, 1, 1, 1)),			  //End colour
			vec3(1, 0, 0),												  //Direction
			3.14159265358979f,											  //Direction variance
			true,														  //GPU based
			m_debugBar);

		m_emitters.push_back(emitter);

		unsigned int light = m_renderer->CreatePointLight(((i < 12) ? vec3(1, 0.5f, 0) : vec3(0, 0.5f, 1)), 12, false);
		m_pieceLights.push_back(light);
	}

	for (unsigned int i = 0; i < 16; ++i)
	{
		unsigned int loc = m_renderer->LoadOBJ("../data/teleporter/teleporter.obj");
		unsigned int light;
		unsigned int emitter;
		if (i < 8)
		{
			m_renderer->LoadTexture("../data/teleporter/teleporter.jpg", loc);
			m_renderer->LoadAmbient("../data/teleporter/teleporter.jpg", loc);
			m_renderer->LoadSpecularMap("../data/teleporter/teleporter.jpg", loc);

			light = m_renderer->CreatePointLight(vec3(0, 0.1f, 1), 14.0f, false);

			emitter = m_renderer->CreateEmitter(1000, 1.0f, 2.0f, 0.1f, 4.0f, 1.0f, 1.0f, vec4(0, 0.5f, 1, 1), vec4(0, 0, 0.5f, 1), vec3(0, 0, -1), 3.141592f / 4.0f, true, m_debugBar);
		}
		else
		{
			m_renderer->LoadTexture("../data/teleporter/teleporterRed.jpg", loc);
			m_renderer->LoadAmbient("../data/teleporter/teleporterRed.jpg", loc);
			m_renderer->LoadSpecularMap("../data/teleporter/teleporterRed.jpg", loc);

			light = m_renderer->CreatePointLight(vec3(1, 0.1f, 0), 14.0f, false);

			emitter = m_renderer->CreateEmitter(1000, 1.0f, 2.0f, 0.1f, 4.0f, 1.0f, 1.0f, vec4(1, 0.5f, 0, 1), vec4(1.0f, 0.0f, 0, 1), vec3(0, 0, 1), 3.141592f / 4.0f, true, m_debugBar);
		}
		m_renderer->SetLightPosition(light, vec3(M_TILE_WIDTH * -3.5f + (i % 8) * M_TILE_WIDTH, 13.0f, M_TILE_WIDTH * (i < 8 ? 4.0f : -4.0f)));
		m_renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * -3.5f + (i % 8) * M_TILE_WIDTH, 0, M_TILE_WIDTH * (i < 8 ? 4.3f : -4.3f))) * glm::scale(vec3(2, 2, 2)), loc);
		m_renderer->SetEmitterPosition(emitter, true,
									   vec3(M_TILE_WIDTH * -3.25f + (i % 8) * M_TILE_WIDTH, 2.0f, M_TILE_WIDTH * (i < 8 ? 4.3f : -4.3f)),
									   vec3(M_TILE_WIDTH * -3.75f + (i % 8) * M_TILE_WIDTH, 13.0f, M_TILE_WIDTH * (i < 8 ? 4.3f : -4.3f)));

		PxBoxGeometry teleporterBox(5.0f, 7.5f, 0.5f);
		PxTransform teleporterPos(PxVec3(M_TILE_WIDTH * -3.5f + (i % 8) * M_TILE_WIDTH, 7.5f, M_TILE_WIDTH * (i < 8 ? 4.3f : -4.3f)));
		PxRigidStatic* teleporterObject = PxCreateStatic(*g_physics, teleporterPos, teleporterBox, *g_physicsMaterial);
		g_physicsScene->addActor(*teleporterObject);
	}

	for (unsigned int i = 0; i < 8; ++i)
	{
		for (unsigned int j = 0; j < 8; ++j)
		{
			m_board[i][j] = -1;
		}
	}

	for (unsigned int i = 0; i < 4; ++i)
	{
		m_renderer->SetEmitterPosition(m_emitters[i], true, vec3(M_TILE_WIDTH * -3.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * -2.5f));
		m_renderer->SetLightPosition(m_pieceLights[i], vec3(M_TILE_WIDTH * -3.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * -2.5f));
		m_board[i * 2][1] = i;
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_renderer->SetEmitterPosition(m_emitters[i + 12], true, vec3(M_TILE_WIDTH * -3.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * 1.5f));
		m_renderer->SetLightPosition(m_pieceLights[i + 12], vec3(M_TILE_WIDTH * -3.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * 1.5f));
		m_board[i * 2][5] = i + 12;
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_renderer->SetEmitterPosition(m_emitters[i + 16], true, vec3(M_TILE_WIDTH * -3.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * 3.5f));
		m_renderer->SetLightPosition(m_pieceLights[i + 16], vec3(M_TILE_WIDTH * -3.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * 3.5f));
		m_board[i * 2][7] = i + 16;
	}

	for (unsigned int i = 0; i < 4; ++i)
	{
		m_renderer->SetEmitterPosition(m_emitters[i + 4], true, vec3(M_TILE_WIDTH * -2.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * -3.5f));
		m_renderer->SetLightPosition(m_pieceLights[i + 4], vec3(M_TILE_WIDTH * -2.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * -3.5f));
		m_board[1 + i * 2][0] = i + 4;
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_renderer->SetEmitterPosition(m_emitters[i + 8], true, vec3(M_TILE_WIDTH * -2.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * -1.5f));
		m_renderer->SetLightPosition(m_pieceLights[i + 8], vec3(M_TILE_WIDTH * -2.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * -1.5f));
		m_board[1 + i * 2][2] = i + 8;
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_renderer->SetEmitterPosition(m_emitters[i + 20], true, vec3(M_TILE_WIDTH * -2.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * 2.5f));
		m_renderer->SetLightPosition(m_pieceLights[i + 20], vec3(M_TILE_WIDTH * -2.5f + i * 2 * M_TILE_WIDTH, 5, M_TILE_WIDTH * 2.5f));
		m_board[1 + i * 2][6] = i + 20;
	}

	m_currentX = 7;
	m_currentY = 7;

	m_positionMarker = m_renderer->LoadOBJ("../data/sphere/sphere.obj");
	m_renderer->LoadTexture("../data/crate.png", m_positionMarker);
	m_renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * 3.5f, 10, M_TILE_WIDTH * 3.5f)), m_positionMarker);
	m_positionLight = m_renderer->CreatePointLight(vec3(1, 1, 1), 15, false);
	m_renderer->SetLightPosition(m_positionLight, vec3(M_TILE_WIDTH * 3.5f, 8, M_TILE_WIDTH * 3.5f));
	m_positionLight2 = m_renderer->CreatePointLight(vec3(1, 1, 1), 15, false);
	m_renderer->SetLightPosition(m_positionLight2, vec3(M_TILE_WIDTH * 3.5f, 12, M_TILE_WIDTH * 3.5f));
	m_colourTimer = 0.1f;




	m_inputTimer = 0;

	m_pieceSelected = -1;

	m_turn = false;

	m_threadFinished = true;
	m_aiDifficulty = 10;
#pragma endregion

	return 0;
}

int CheckersTest2::Deinit()
{
	delete g_playerCollisions;
	delete m_mover;

	return PhysicsBase::Deinit();
}

void CheckersTest2::Update(float a_deltaTime)
{
	PhysicsBase::Update(a_deltaTime);

	m_camera->Update(a_deltaTime);
	m_renderer->UpdateAnimation(m_animationTimer, m_animatedModel);
	m_animationTimer += a_deltaTime;

	CheckersUpdate(a_deltaTime);

#pragma region Checkers Mover
	
	m_mover->Update()

#pragma endregion

#pragma region Player Movement
	glm::mat4 cameraWorld = m_camera->GetWorldTransform();
	//m_player->setKinematicTarget(PxTransform(PxVec3(cameraWorld[3].x, cameraWorld[3].y, cameraWorld[3].z)));
	PxVec3 displacement = PxVec3(0, 0, 0);
	bool notZero = false;
	if (InputManager::GetKey(Keys::W))
	{
		displacement -= PxVec3(cameraWorld[2].x, 0, cameraWorld[2].z);
		notZero = true;
	}
	if (InputManager::GetKey(Keys::A))
	{
		displacement -= PxVec3(cameraWorld[0].x, 0, cameraWorld[0].z);
		notZero = true;
	}
	if (InputManager::GetKey(Keys::S))
	{
		displacement += PxVec3(cameraWorld[2].x, 0, cameraWorld[2].z);
		notZero = true;
	}
	if (InputManager::GetKey(Keys::D))
	{
		displacement += PxVec3(cameraWorld[0].x, 0, cameraWorld[0].z);
		notZero = true;
	}
	if (notZero)
		displacement = displacement.getNormalized() * m_walkSpeed;

	if (InputManager::GetKey(Keys::SPACE))
	{
		PxControllerState state;
		g_playerController->getState(state);	
		if (state.collisionFlags > 0)
			m_verticleSpeed = 0.5f;
	}

	if (m_verticleSpeed > -10.0f)
		m_verticleSpeed -= a_deltaTime;

	displacement.y = m_verticleSpeed;

	PxControllerFilters filters;
	g_playerController->move(displacement, 0.01f, a_deltaTime, filters);

	PxExtendedVec3 playerPos = g_playerController->getPosition();
	PxExtendedVec3 footPos = g_playerController->getFootPosition();
	//I do these calculations individually inside this vector constructor because PxEtendedVec3 doesn't contain some of the necessary operators to do this.
	vec3 endPos = vec3(2.0f * playerPos.x - footPos.x, 2.0f * playerPos.y - footPos.y, 2.0f * playerPos.z - footPos.z);
	m_camera->SetPosition(endPos);
#pragma endregion

#pragma region Spawn Physics Props
	m_spawnTimer += a_deltaTime;

	if (m_spawnTimer >= 1.0f)
	{
		vec3 randPos = vec3(((float)rand() / (float)RAND_MAX) * 20.0f + 200, ((float)rand() / (float)RAND_MAX) * 20.0f + 200.0f, ((float)rand() / (float)RAND_MAX) * 20.0f);

		if (rand() % 2 == 0)
			AddBox(g_physicsMaterial, 10.0f, vec3(2.0f, 2.0f, 2.0f), randPos, true);
		else
			AddSphere(g_physicsMaterial, 10.0f, 2.0f, randPos, true);

		m_physicsLights.push_back(m_renderer->CreatePointLight(vec3(((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX)), 20, false, randPos));

		m_spawnTimer = 0;
	}
#pragma endregion

#pragma region Shooting
	m_shootTimer -= a_deltaTime;

	cameraWorld[3] = vec4(endPos, 1);
	m_renderer->SetTransform(glm::translate(glm::scale(glm::rotate(cameraWorld, 0.2f, vec3(1, 0, 0)), vec3(0.005f, 0.005f, 0.005f)), vec3(0.0f, -25.0f, -5.0f)), m_gun);
	if (m_shootTimer > 0.0f)
	{
		//Move the gun back if reloading.
		m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_gun), vec3(0, 0, 5 * m_shootTimer)), m_gun);
	}

	if (InputManager::GetMouseDown(0) && !InputManager::GetMouseDown(1) && m_shootTimer < 0.0f)
	{
		//Shoot
		m_shootTimer = 0.5f;
		AddSphere(g_physicsMaterial, 10.0f, 2.0f, vec3(cameraWorld[3]) + vec3(cameraWorld[2]) * -5.0f, true);
		vec3 forward = glm::rotateY(vec3(cameraWorld[2]), -0.2f);
		((PxRigidDynamic*)(g_physicsActors[g_physicsActors.size() - 1]))->setLinearVelocity(PxVec3(forward.x, forward.y, forward.z) * -1 * m_shootForce);

		m_physicsLights.push_back(m_renderer->CreatePointLight(vec3(((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX)), 20, false, vec3(cameraWorld[3])));
	}
#pragma endregion

#pragma region Update Physics Lights
	for (unsigned int i = 0; i < m_physicsLights.size(); ++i)
	{
		if (m_physicsLights[i] == -1)
			continue;

		PxRigidActor* actor = g_physicsActors[i];
		if (actor->isRigidDynamic())
		{
			if (((PxRigidDynamic*)actor)->isSleeping())
				continue;
		}

		PxU32 shapesIndex = actor->getNbShapes();
		PxShape** shapes = new PxShape*[shapesIndex];

		actor->getShapes(shapes, shapesIndex);
		--shapesIndex;

		PxMat44 m(PxShapeExt::getGlobalPose(*shapes[shapesIndex], *actor));
		glm::mat4 transform(m.column0.x, m.column0.y, m.column0.z, m.column0.w,
			m.column1.x, m.column1.y, m.column1.z, m.column1.w,
			m.column2.x, m.column2.y, m.column2.z, m.column2.w,
			m.column3.x, m.column3.y, m.column3.z, m.column3.w);

		m_renderer->SetLightPosition(m_physicsLights[i], vec3(transform[3]));

		delete[] shapes;

	}
#pragma endregion
}

void CheckersTest2::Draw()
{
	PhysicsBase::Draw();
}

#pragma region Checkers Functions

void CheckersTest2::CheckersUpdate(float a_deltaTime)
{
	m_inputTimer -= a_deltaTime;
	m_colourTimer -= a_deltaTime;

	if (m_colourTimer <= 0)
	{
		//Randomise the colour
		vec3 randomColour((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
		m_renderer->SetLightColour(m_positionLight, randomColour);
		m_renderer->SetLightColour(m_positionLight2, randomColour);
		m_colourTimer = 0.1f;
	}

	if (m_inputTimer < 0)
	{
		if (InputManager::GetKey(Keys::LEFT))
		{
			if (m_currentX > 0)
			{
				--m_currentX;
				m_inputTimer = 0.15f;
				m_renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY)), m_positionMarker);
				m_renderer->SetLightPosition(m_positionLight, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 8, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
				m_renderer->SetLightPosition(m_positionLight2, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 12, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
			}
		}
		if (InputManager::GetKey(Keys::RIGHT))
		{
			if (m_currentX < 7)
			{
				++m_currentX;
				m_inputTimer = 0.15f;
				m_renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY)), m_positionMarker);
				m_renderer->SetLightPosition(m_positionLight, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 8, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
				m_renderer->SetLightPosition(m_positionLight2, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 12, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
			}
		}
		if (InputManager::GetKey(Keys::UP))
		{
			if (m_currentY > 0)
			{
				--m_currentY;
				m_inputTimer = 0.15f;
				m_renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY)), m_positionMarker);
				m_renderer->SetLightPosition(m_positionLight, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 8, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
				m_renderer->SetLightPosition(m_positionLight2, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 12, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
			}
		}
		if (InputManager::GetKey(Keys::DOWN))
		{
			if (m_currentY < 7)
			{
				++m_currentY;
				m_inputTimer = 0.15f;
				m_renderer->SetTransform(glm::translate(vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY)), m_positionMarker);
				m_renderer->SetLightPosition(m_positionLight, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 8, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
				m_renderer->SetLightPosition(m_positionLight2, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentX, 12, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * m_currentY));
			}
		}
		if (!m_turn)
		{
			if (InputManager::GetKey(Keys::ENTER))
			{
				HandleEnter(m_board, m_currentX, m_currentY, m_previousX, m_previousY, m_turn, true, m_pieceSelected);
				m_inputTimer = 0.15f;
			}
		}
		else if (m_threadFinished)
		{
			std::thread thr(&CheckersTest2::UseAIMove, this);
			std::swap(thr, m_aiThread);
			m_aiThread.detach();
		}
		else if (InputManager::GetKey(Keys::ENTER))
		{
			std::cout << "Invalid Move: Must Wait For Computer Opponents Move!" << std::endl;
			m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
			m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
			m_colourTimer = 0.5f;
			m_inputTimer = 0.15f;
		}
	}
}

void CheckersTest2::HandleEnter(int(&a_board)[8][8], const unsigned int a_xPos, const unsigned int a_yPos, unsigned int &a_prevX, unsigned int &a_prevY, bool &a_turn, const bool a_changeEmitters, unsigned int& a_pieceSelected)
{
	if (a_pieceSelected == -1)
	{
		if (a_board[a_xPos][a_yPos] != -1)
		{
			if ((a_turn && a_board[a_xPos][a_yPos] < 12) || (!a_turn && a_board[a_xPos][a_yPos] >= 12))
			{
				//Teleporters
				if ((a_turn && a_yPos == 7) || (!a_turn && a_yPos == 0))
				{
					//If telefragging
					if (a_changeEmitters && ((a_board[a_xPos + 1][0] != -1 && a_turn) || (a_board[a_xPos - 1][7] != -1 && !a_turn)))
					{
						if (a_turn)
						{
							m_renderer->SetEmitterPosition(m_emitters[a_board[a_xPos + 1][0]], true, vec3(M_TILE_WIDTH * 5.5f * ((a_board[a_xPos + 1][0] < 12) ? -1 : 1), 5, 0));
							m_renderer->SetLightPosition(m_pieceLights[a_board[a_xPos + 1][0]], vec3(M_TILE_WIDTH * 5.5f * ((a_board[a_xPos + 1][0] < 12) ? -1 : 1), 5, 0));
						}
						else
						{
							m_renderer->SetEmitterPosition(m_emitters[a_board[a_xPos - 1][7]], true, vec3(M_TILE_WIDTH * 5.5f * ((a_board[a_xPos - 1][7] < 12) ? -1 : 1), 5, 0));
							m_renderer->SetLightPosition(m_pieceLights[a_board[a_xPos - 1][7]], vec3(M_TILE_WIDTH * 5.5f * ((a_board[a_xPos - 1][7] < 12) ? -1 : 1), 5, 0));
						}
					}

					if (a_turn)
					{
						a_board[a_xPos + 1][0] = a_board[a_xPos][7];
						a_board[a_xPos][7] = -1;
						m_renderer->SetEmitterPosition(m_emitters[a_board[a_xPos + 1][0]], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * (a_xPos + 1), 5, 0));
						m_renderer->SetLightPosition(m_pieceLights[a_board[a_xPos + 1][0]], vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * (a_xPos + 1), 5, 0));

					}
					else
					{
						a_board[a_xPos - 1][7] = a_board[a_xPos][0];
						a_board[a_xPos][0] = -1;
						m_renderer->SetEmitterPosition(m_emitters[a_board[a_xPos - 1][7]], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * (a_xPos - 1), 5, M_TILE_WIDTH * 3.5f));
						m_renderer->SetLightPosition(m_pieceLights[a_board[a_xPos - 1][7]], vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * (a_xPos - 1), 5, M_TILE_WIDTH * 3.5f));
					}
					a_turn = !a_turn;
				}
				//For an explanation of the below 'if' statement, look this way -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> v v v v v v v v v v v v v v v v v v v v v
				else if ((a_turn &&																																				//If it is player 1's turn...
					((a_xPos < 6 && a_yPos < 6 && a_board[a_xPos + 1][a_yPos + 1] >= 12 && a_board[a_xPos + 2][a_yPos + 2] == -1) ||											//...and he can jump forwards and to one side...
					(a_xPos > 1 && a_yPos < 6 && a_board[a_xPos - 1][a_yPos + 1] >= 12 && a_board[a_xPos - 2][a_yPos + 2] == -1)))												//...or forwards and to the other side...
					||																																							// OR
					(!a_turn &&																																					//If it is player 2's turn...
					((a_xPos < 6 && a_yPos > 1 && a_board[a_xPos + 1][a_yPos - 1] < 12 && a_board[a_xPos + 1][a_yPos - 1] != -1 && a_board[a_xPos + 2][a_yPos - 2] == -1) ||	//...and he can jump backwards and to one side...
					(a_xPos > 1 && a_yPos > 1 && a_board[a_xPos - 1][a_yPos - 1] < 12 && a_board[a_xPos - 1][a_yPos - 1] != -1 && a_board[a_xPos - 2][a_yPos - 2] == -1))))		//...or backwards and to the other side... 
				{
					//Then this piece can jump, do a move.
					a_pieceSelected = a_board[a_xPos][a_yPos];
					a_prevX = a_xPos;
					a_prevY = a_yPos;
					if (a_changeEmitters)
					{
						m_renderer->SetEmitterPosition(m_emitters[a_pieceSelected], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_xPos, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_yPos));
						m_renderer->SetLightPosition(m_pieceLights[a_pieceSelected], vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_xPos, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_yPos));
					}
				}
				else
				{
					//This piece cannot capture, check to see if any other can.
					//Check for if a capture-move is possible
					for (unsigned int i = 0; i < 8; ++i)
					{
						for (unsigned int j = 0; j < 8; ++j)
						{
							//Check to see if player 1 can take player 2.
							if (a_turn)
							{
								if (a_board[i][j] != -1 && a_board[i][j] < 12)
								{
									//Check each direction for opponent pieces
									//Whoops- made it check backwards as well as forwards. Commented out lines should only apply when kings are involved.
									if ((i < 6 && j < 6 && a_board[i + 1][j + 1] >= 12 && a_board[i + 2][j + 2] == -1) ||
										/*(i < 6 && j > 1 && m_board[i + 1][j - 1] >= 12 && m_board[i + 2][j - 2] == -1) ||*/
										(i > 1 && j < 6 && a_board[i - 1][j + 1] >= 12 && a_board[i - 2][j + 2] == -1) /*||
																													   (i > 1 && j > 1 && m_board[i - 1][j - 1] >= 12 && m_board[i - 2][j - 2] == -1)*/)
									{
										std::cout << "Invalid Move: Must Capture Opponent Piece!" << std::endl;
										m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
										m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
										m_colourTimer = 0.5f;
										return;
									}
								}
							}
							//Check to see if player 2 can take player 1
							else
							{
								if (a_board[i][j] >= 12)
								{
									//Check each direction for opponent pieces
									//Whoops- made it check backwards as well as forwards. Commented out lines should only apply when kings are involved.
									if (//(i < 6 && j < 6 && m_board[i + 1][j + 1] < 12 && m_board[i + 1][j - 1] != -1 && m_board[i + 2][j + 2] == -1) ||
										(i < 6 && j > 1 && a_board[i + 1][j - 1] < 12 && a_board[i + 1][j - 1] != -1 && a_board[i + 2][j - 2] == -1) ||
										//(i > 1 && j < 6 && m_board[i - 1][j + 1] < 12 && m_board[i - 1][j - 1] != -1 && m_board[i - 2][j + 2] == -1) ||
										(i > 1 && j > 1 && a_board[i - 1][j - 1] < 12 && a_board[i - 1][j - 1] != -1 && a_board[i - 2][j - 2] == -1))
									{
										std::cout << "Invalid Move: Must Capture Opponent Piece!" << std::endl;
										m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
										m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
										m_colourTimer = 0.5f;
										return;
									}
								}
							}
						}
					}

					//If we are here, this piece cannot jump, but neither can any other.
					a_pieceSelected = a_board[a_xPos][a_yPos];
					a_prevX = a_xPos;
					a_prevY = a_yPos;
					if (a_changeEmitters)
					{
						m_renderer->SetEmitterPosition(m_emitters[a_pieceSelected], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_xPos, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_yPos));
						m_renderer->SetLightPosition(m_pieceLights[a_pieceSelected], vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_xPos, 10, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_yPos));
					}

				}
			}
			else
			{
				std::cout << "Invalid Move: Cannot Select Opponents Piece!" << std::endl;
				m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
				m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
				m_colourTimer = 0.5f;
			}
		}
	}
	else
	{
		if (ValidMove(a_board, a_xPos, a_yPos, a_prevX, a_prevY, a_turn, a_changeEmitters))
		{
			if (a_xPos == a_prevX + 2 || a_xPos == a_prevX - 2)	//If the move was a capture one
			{
				unsigned int takenSquare = a_board[(a_xPos + a_prevX) / 2][(a_yPos + a_prevY) / 2];

				if (a_changeEmitters)
				{
					m_renderer->SetEmitterPosition(m_emitters[takenSquare], true, vec3(M_TILE_WIDTH * 5.5f * ((takenSquare < 12) ? -1 : 1), 5, 0));
					m_renderer->SetLightPosition(m_pieceLights[takenSquare], vec3(M_TILE_WIDTH * 5.5f * ((takenSquare < 12) ? -1 : 1), 5, 0));
				}

				a_board[(a_xPos + a_prevX) / 2][(a_yPos + a_prevY) / 2] = -1;
			}
			if (a_changeEmitters)
			{
				m_renderer->SetEmitterPosition(m_emitters[a_pieceSelected], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_xPos, 5, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_yPos));
				m_renderer->SetLightPosition(m_pieceLights[a_pieceSelected], vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_xPos, 5, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_yPos));
			}
			a_board[a_prevX][a_prevY] = -1;
			a_board[a_xPos][a_yPos] = a_pieceSelected;
			a_pieceSelected = -1;
			a_turn = !a_turn;
		}
		else
		{
			if (a_changeEmitters)
			{
				m_renderer->SetEmitterPosition(m_emitters[a_pieceSelected], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_prevX, 5, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_prevY));
				m_renderer->SetLightPosition(m_pieceLights[a_pieceSelected], vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_prevX, 5, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * a_prevY));
			}
			a_pieceSelected = -1;
		}
	}
}

bool CheckersTest2::ValidMove(const int a_board[8][8], const unsigned int a_xPos, const unsigned int a_yPos, const unsigned int a_prevX, const unsigned int a_prevY, const bool a_turn, bool a_changeEmitters)
{
	//I should later account for kings or whatever the upgraded checker pieces are called.

	//Remember to revise my code at some point to make sure that < 12 checks aren't breaking everything by selecting -1 tiles.

	//Capture move
	if ((a_xPos == a_prevX + 2 || a_xPos == a_prevX - 2) && ((a_yPos == a_prevY + 2 && a_turn) || (a_yPos == a_prevY - 2 && !a_turn)))
	{
		if (a_board[a_xPos][a_yPos] != -1)
		{
			if (a_changeEmitters)
			{
				std::cout << "Invalid Move: Square Occupied!" << std::endl;
				m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
				m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
				m_colourTimer = 0.5f;
			}
			return false;
		}
		unsigned int takenSquare = a_board[(a_xPos + a_prevX) / 2][(a_yPos + a_prevY) / 2];
		if (takenSquare == -1)
		{
			if (a_changeEmitters)
			{
				std::cout << "Invalid Move: Jump Move Requires Piece To Jump!" << std::endl;
				m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
				m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
				m_colourTimer = 0.5f;
			}
			return false;
		}
		if ((takenSquare < 12) == (a_board[a_prevX][a_prevY] < 12))
		{
			if (a_changeEmitters)
			{
				std::cout << "Invalid Move: Jumped Piece Must Belong To Opponent" << std::endl;
				m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
				m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
				m_colourTimer = 0.5f;
			}
			return false;
		}
		return true;
	}
	else
	{
		//First of all, do a check to see if a capture move is possible- if it is, then tell the player that their move is invalid, because 'take a piece' moves have to be done.
		//I only check if a capture move is possible for the current piece, because checks for all other pieces have already been done.
		//For an explanation of the below 'if' statement, look this way -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> 	v v v v v v v v v v v v v v v v v v v v v
		if ((a_turn &&																																							//If it is player 1's turn...
			((a_prevX < 6 && a_prevY < 6 && a_board[a_prevX + 1][a_prevY + 1] >= 12 && a_board[a_prevX + 2][a_prevY + 2] == -1) ||												//...and he can jump forwards and to one side...
			(a_prevX > 1 && a_prevY < 6 && a_board[a_prevX - 1][a_prevY + 1] >= 12 && a_board[a_prevX - 2][a_prevY + 2] == -1)))												//...or forwards and to the other side...
			||																																									// OR
			(!a_turn &&																																							//If it is player 2's turn...
			((a_prevX < 6 && a_prevY > 1 && a_board[a_prevX + 1][a_prevY - 1] < 12 && a_board[a_prevX + 1][a_prevY - 1] != -1 && a_board[a_prevX + 2][a_prevY - 2] == -1) ||	//...and he can jump backwards and to one side...
			(a_prevX > 1 && a_prevY > 1 && a_board[a_prevX - 1][a_prevY - 1] < 12 && a_board[a_prevX - 1][a_prevY - 1] != -1 && a_board[a_prevX - 2][a_prevY - 2] == -1))))	//...or backwards and to the other side... 
		{
			if (a_changeEmitters)
			{
				std::cout << "Invalid Move: Must Capture Opponent Piece!" << std::endl;
				m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
				m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
				m_colourTimer = 0.5f;
			}
			return false;
		}

		//Default move
		if ((a_xPos == a_prevX + 1 || a_xPos == a_prevX - 1) && ((a_yPos == a_prevY + 1 && a_turn) || (a_yPos == a_prevY - 1 && !a_turn)))
		{
			if (a_board[a_xPos][a_yPos] != -1)
			{
				if (a_changeEmitters)
				{
					std::cout << "Invalid Move: Square Occupied!" << std::endl;
					m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
					m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
					m_colourTimer = 0.5f;
				}
				return false;
			}
			return true;
		}
		if (a_changeEmitters)
		{
			std::cout << "Invalid Move!" << std::endl;
			m_renderer->SetLightColour(m_positionLight, vec3(1, 0, 0));
			m_renderer->SetLightColour(m_positionLight2, vec3(1, 0, 0));
			m_colourTimer = 0.5f;
		}
		return false;
	}

}

void CheckersTest2::AIMove(int(&a_board)[8][8], const bool a_turn, const unsigned int a_difficulty)
{
	std::vector<std::vector<std::vector<int>>> actions;
	std::vector<int> scores;

	int cloneBoard[8][8];
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			cloneBoard[i][j] = a_board[i][j];
		}
	}

	actions = GetPossibleMoves(cloneBoard, a_turn);

	//If there is only one action, do that action.
	if (actions.size() == 1)
	{
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				a_board[i][j] = actions[0][i][j];
			}
		}
		return;
	}


	scores.assign(actions.size(), 0);
	for (unsigned int i = 0; i < actions.size(); ++i)
	{
		for (unsigned int j = 0; j < a_difficulty; ++j)
		{
			scores[i] -= PlayUntilEnd(actions[i], !a_turn);
		}
	}

	int highestScore = -99999;
	int highestScoreIndex = -1;
	for (unsigned int i = 0; i < scores.size(); ++i)
	{
		if (scores[i] > highestScore)
		{
			highestScore = scores[i];
			highestScoreIndex = i;
		}
	}

	if (highestScoreIndex != -1)
	{
		for (unsigned int i = 0; i < 8; ++i)
		{
			for (unsigned int j = 0; j < 8; ++j)
			{
				a_board[i][j] = actions[highestScoreIndex][i][j];
			}
		}
	}
}

std::vector<std::vector<std::vector<int>>> CheckersTest2::GetPossibleMoves(const int a_board[8][8], const bool a_turn)
{
	std::vector<std::vector<std::vector<int>>> results;

	int captureMovePossible = -1;
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			//Capture move possible
			//For an explanation of the below 'if' statement, look this way -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> -> ->   v v v v v v v v v v v v v v v v v v v v v
			if ((a_turn &&	a_board[i][j] != -1 && a_board[i][j] < 12 &&														//If it is player 1's turn...
				((i < 6 && j < 6 && a_board[i + 1][j + 1] >= 12 && a_board[i + 2][j + 2] == -1) ||								//...and he can jump forwards and to one side...
				(i > 1 && j < 6 && a_board[i - 1][j + 1] >= 12 && a_board[i - 2][j + 2] == -1)))								//...or forwards and to the other side...
				||																												// OR
				(!a_turn &&	a_board[i][j] >= 12 &&																				//If it is player 2's turn...
				((i < 6 && j > 1 && a_board[i + 1][j - 1] < 12 && a_board[i + 1][j - 1] != -1 && a_board[i + 2][j - 2] == -1) ||//...and he can jump backwards and to one side...
				(i > 1 && j > 1 && a_board[i - 1][j - 1] < 12 && a_board[i - 1][j - 1] != -1 && a_board[i - 2][j - 2] == -1))))	//...or backwards and to the other side... 
			{
				captureMovePossible = i * 8 + j;
				goto outsideLoop;	//This goes here	-->--->--->---->---->--->--|
			}																	 /*|*/
		}																		 /*|*/
	}																			 /*|*/
	//This outsideLoop bit is used for breaking out of the nested for loops.	   |
outsideLoop: //<--<--<--<--<--<--<---<--<--<--<--<--<--<--<--<--<--<--<--<--<--<

	for (int i = (captureMovePossible == -1 ? 0 : captureMovePossible / 8); i < 8; ++i)
	{
		for (int j = (captureMovePossible == -1 ? 0 : captureMovePossible % 8); j < 8; ++j)
		{
			//If the tile (i, j) contains a piece that belongs to the player whos turn it is
			if (a_board[i][j] != -1 && a_board[i][j] < 12 == a_turn)
			{
				int moveDirection = (a_turn ? 1 : -1);

				//Normal Move
				if (captureMovePossible == -1 && i < 7 && j + moveDirection < 8 && j + moveDirection >= 0 && ValidMove(a_board, i + 1, j + moveDirection, i, j, a_turn, false))
				{
					std::vector<std::vector<int>> newResult;
					std::vector<int> emptyRow;
					emptyRow.assign(8, -1);
					newResult.assign(8, emptyRow);

					for (unsigned int k = 0; k < 8; ++k)
					{
						for (unsigned int l = 0; l < 8; ++l)
						{
							newResult[k][l] = a_board[k][l];
						}
					}
					//Move the piece.
					newResult[i + 1][j + moveDirection] = newResult[i][j];
					newResult[i][j] = -1;

					results.push_back(newResult);
				}
				//Normal Move other direction
				if (captureMovePossible == -1 && i > 0 && j + moveDirection < 8 && j + moveDirection >= 0 && ValidMove(a_board, i - 1, j + moveDirection, i, j, a_turn, false))
				{
					std::vector<std::vector<int>> newResult;
					std::vector<int> emptyRow;
					emptyRow.assign(8, -1);
					newResult.assign(8, emptyRow);

					for (unsigned int k = 0; k < 8; ++k)
					{
						for (unsigned int l = 0; l < 8; ++l)
						{
							newResult[k][l] = a_board[k][l];
						}
					}
					//Move the piece.
					newResult[i - 1][j + moveDirection] = newResult[i][j];
					newResult[i][j] = -1;

					results.push_back(newResult);
				}
				//Jump Move
				if (captureMovePossible != -1 && i < 6 && j + moveDirection < 7 && j + moveDirection > 0 && ValidMove(a_board, i + 2, j + moveDirection * 2, i, j, a_turn, false))
				{
					std::vector<std::vector<int>> newResult;
					std::vector<int> emptyRow;
					emptyRow.assign(8, -1);
					newResult.assign(8, emptyRow);

					for (unsigned int k = 0; k < 8; ++k)
					{
						for (unsigned int l = 0; l < 8; ++l)
						{
							newResult[k][l] = a_board[k][l];
						}
					}
					//Move the piece, and remove the taken piece.
					newResult[i + 2][j + moveDirection * 2] = newResult[i][j];
					newResult[i][j] = -1;
					newResult[i + 1][j + moveDirection] = -1;

					results.push_back(newResult);
				}
				//Jump Move other direction
				if (captureMovePossible != -1 && i > 1 && j + moveDirection < 7 && j + moveDirection > 0 && ValidMove(a_board, i - 2, j + moveDirection * 2, i, j, a_turn, false))
				{
					std::vector<std::vector<int>> newResult;
					std::vector<int> emptyRow;
					emptyRow.assign(8, -1);
					newResult.assign(8, emptyRow);

					for (unsigned int k = 0; k < 8; ++k)
					{
						for (unsigned int l = 0; l < 8; ++l)
						{
							newResult[k][l] = a_board[k][l];
						}
					}
					//Move the piece, and remove the taken piece.
					newResult[i - 2][j + moveDirection * 2] = newResult[i][j];
					newResult[i][j] = -1;
					newResult[i - 1][j + moveDirection] = -1;

					results.push_back(newResult);
				}
				//Teleport Move
				if (captureMovePossible == -1 && ((a_turn && j == 7) || (!a_turn && j == 0)))
				{
					std::vector<std::vector<int>> newResult;
					std::vector<int> emptyRow;
					emptyRow.assign(8, -1);
					newResult.assign(8, emptyRow);

					for (unsigned int k = 0; k < 8; ++k)
					{
						for (unsigned int l = 0; l < 8; ++l)
						{
							newResult[k][l] = a_board[k][l];
						}
					}

					if (a_turn)
					{
						newResult[i + 1][0] = newResult[i][7];
						newResult[i][7] = -1;
					}
					else
					{
						newResult[i - 1][7] = newResult[i][0];
						newResult[i][0] = -1;
					}

					results.push_back(newResult);
				}
			}
		}
	}
	return results;
}

int CheckersTest2::PlayUntilEnd(std::vector<std::vector<int>> a_board, const bool a_turn)
{
	int board[8][8];
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			board[i][j] = a_board[i][j];
		}
	}

	bool turn = a_turn;

	std::vector<std::vector<std::vector<int>>> possibleMoves = GetPossibleMoves(board, turn);

	while (possibleMoves.size() > 0)
	{
		//Do random moves until the game ends.
		int randChoice = rand() % possibleMoves.size();

		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				board[i][j] = possibleMoves[randChoice][i][j];
			}
		}

		turn = !turn;

		possibleMoves = GetPossibleMoves(board, turn);
	}
	//If win, return 1
	//If draw, return 0
	//If lose, return -1
	bool player1Present = false;
	bool player2Present = false;
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			if (board[i][j] != -1 && board[i][j] < 12)
			{
				player1Present = true;
			}
			else if (board[i][j] >= 12)
			{
				player2Present = true;
			}
		}
	}
	if ((player1Present && player2Present) || (!player1Present && !player2Present))
	{
		return 0;
	}
	else if (player1Present)
	{
		return (a_turn ? 1 : -1);
	}
	else
	{
		return (a_turn ? -1 : 1);
	}

}

void CheckersTest2::UseAIMove()
{
	m_threadFinished = false;
	std::vector<bool> emittersMoved;
	emittersMoved.assign(24, false);

	AIMove(m_board, m_turn, m_aiDifficulty);
	m_turn = !m_turn;
	//Move pieces
	for (unsigned int i = 0; i < 8; ++i)
	{
		for (unsigned int j = 0; j < 8; ++j)
		{
			if (m_board[i][j] != -1)
			{
				m_renderer->SetEmitterPosition(m_emitters[m_board[i][j]], true, vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * i, 5, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * j));
				m_renderer->SetLightPosition(m_pieceLights[m_board[i][j]], vec3(M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * i, 5, M_TILE_WIDTH * -3.5f + M_TILE_WIDTH * j));
				emittersMoved[m_board[i][j]] = true;
			}
		}
	}

	//Remove taken pieces
	for (unsigned int i = 0; i < emittersMoved.size(); ++i)
	{
		if (!emittersMoved[i])
		{
			m_renderer->SetEmitterPosition(m_emitters[i], true, vec3(M_TILE_WIDTH * 5.5f * ((m_emitters[i] < 12) ? -1 : 1), 5, 0));
			m_renderer->SetLightPosition(m_pieceLights[i], vec3(M_TILE_WIDTH * 5.5f * ((m_emitters[i] < 12) ? -1 : 1), 5, 0));
		}
	}

	m_threadFinished = true;
}

#pragma endregion