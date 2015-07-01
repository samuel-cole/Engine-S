#include "PhysicsConcrete1.h"
#include "Renderer.h"
#include "WalkCamera.h"
#include "InputManager.h"
//Used for addlight function
#include "tut13\Tutorial13.h"

#include <iostream>

int PhysicsConcrete1::Init()
{
	int baseInit = PhysicsBase::Init();
	if (baseInit != 0)
		return baseInit;

	g_physicsMaterial = g_physics->createMaterial(0.5f, 0.5f, 0.5f);

	//Add a plane
	//PxTransform pose = PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(PxHalfPi * 1.0f, PxVec3(0.0f, 0.0f, 1.0f)));
	//PxRigidStatic* plane = PxCreateStatic(*g_physics, pose, PxPlaneGeometry(), *g_physicsMaterial);
	//g_physicsScene->addActor(*plane);

	m_gun = m_renderer->LoadOBJ("../data/gun/crossbow.obj");
	m_renderer->LoadTexture("../data/gun/texture.jpg", m_gun);
	m_renderer->LoadAmbient("../data/gun/texture.jpg", m_gun);
	m_renderer->LoadSpecularMap("../data/gun/specular.jpg", m_gun);

	m_spawnTimer = 0.0f;
	m_shootTimer = 0.0f;
	m_shootForce = 100.0f;

	//unsigned int object = AddProceduralPlane(99, 100, 8, vec3(0, 0, 0), g_physicsMaterial, 100);
	unsigned int object = AddProceduralPlane(99, 9, 9, vec3(0, -300, 0), g_physicsMaterial, 100);
	m_renderer->LoadTexture("../data/checkerboard.png", object);
	m_renderer->LoadAmbient("../data/checkerboard.png", object);

	TwAddVarRW(m_debugBar, "Shoot Force", TW_TYPE_FLOAT, &m_shootForce, "");

	TwAddSeparator(m_debugBar, "Lights", "");
	TwAddButton(m_debugBar, "AddLight", AddLight, (void*)(m_renderer), "");

	PxBoxGeometry playerBox(1, 2, 1);
	PxTransform playerPos(PxVec3(0, 0, 0));
	m_player = PxCreateDynamic(*g_physics, playerPos, playerBox, *g_physicsMaterial, 10);
	m_player->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	g_physicsScene->addActor(*m_player);

	return 0;
}

int PhysicsConcrete1::Deinit()
{
	return PhysicsBase::Deinit();
}

void PhysicsConcrete1::Update(float a_deltaTime)
{
	PhysicsBase::Update(a_deltaTime);

	m_spawnTimer += a_deltaTime;
	m_shootTimer -= a_deltaTime;

	glm::mat4 cameraWorld = m_camera->GetWorldTransform();
	m_player->setKinematicTarget(PxTransform(PxVec3(cameraWorld[3].x, cameraWorld[3].y, cameraWorld[3].z)));

	if (m_spawnTimer >= 1.0f)
	{
		std::cout << m_models.size() << std::endl;

		vec3 randPos = vec3(((float)rand() / (float)RAND_MAX) * 20.0f, ((float)rand() / (float)RAND_MAX) * 20.0f + 200.0f, ((float)rand() / (float)RAND_MAX) * 20.0f);

		if (rand() % 2 == 0)
			AddBox(g_physicsMaterial, 10.0f, vec3(2.0f, 2.0f, 2.0f), randPos, true);
		else
			AddSphere(g_physicsMaterial, 10.0f, 2.0f, randPos, true);

		m_lights.push_back(m_renderer->CreatePointLight(vec3(((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX)), 20, false, randPos));

		m_spawnTimer = 0;
	}

	//Shooting and moving the gun should happen after updating the camera.
	m_renderer->SetTransform(glm::translate(glm::scale(glm::rotate(cameraWorld, 0.2f, vec3(1, 0, 0)), vec3(0.005f, 0.005f, 0.005f)), vec3(0.0f, -25.0f, -5.0f)), m_gun);
	if (m_shootTimer > 0.0f)
	{
		//Move the gun back if reloading.
		m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_gun), vec3(0, 0, 5 * m_shootTimer)), m_gun);
	}

	if (InputManager::GetMouseDown(0) && m_shootTimer < 0.0f)
	{
		//Shoot
		m_shootTimer = 0.5f;
		AddSphere(g_physicsMaterial, 10.0f, 2.0f, vec3(cameraWorld[3]) + vec3(cameraWorld[2]) * -5.0f, true);
		vec3 forward = glm::rotateY(vec3(cameraWorld[2]), -0.2f);
		((PxRigidDynamic*)(g_physicsActors[g_physicsActors.size() - 1]))->setLinearVelocity(PxVec3(forward.x, forward.y, forward.z) * -1 * m_shootForce);

		m_lights.push_back(m_renderer->CreatePointLight(vec3(((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX)), 20, false, vec3(cameraWorld[3])));
	}

	for (unsigned int i = 0; i < m_lights.size(); ++i)
	{
		if (m_lights[i] == -1)
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

		m_renderer->SetLightPosition(m_lights[i], vec3(transform[3]));

		delete[] shapes;

	}
}

void PhysicsConcrete1::Draw()
{
	PhysicsBase::Draw();
}