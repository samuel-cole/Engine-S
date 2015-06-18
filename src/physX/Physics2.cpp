#include "Physics2.h"
#include "glm\ext.hpp"
#include "FlyCamera.h"
#include "Renderer.h"
#include "InputManager.h"
//This is included for its Add Light function.
#include "tut13\Tutorial13.h"


#include <iostream>


class myAllocator : public PxAllocatorCallback
{
public:
	virtual ~myAllocator() {}
	virtual void* allocate(size_t a_size, const char* a_typeName, const char* a_fileName, int a_line)
	{
		void* pointer = _aligned_malloc(a_size, 16);
		return pointer;
	}
	virtual void deallocate(void* a_ptr)
	{
		_aligned_free(a_ptr);
	}
};

int Physics2::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);

	SetUpPhysX();

	//Add a plane
	PxTransform pose = PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(PxHalfPi * 1.0f, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidStatic* plane = PxCreateStatic(*g_physics, pose, PxPlaneGeometry(), *g_physicsMaterial);
	g_physicsScene->addActor(*plane);

	//for (int i = 0; i < 20; ++i)
	//{
	//	AddBox(g_boxMaterial, 10, vec3(8, 8, 8), vec3(i * 16, 8, 0));
	//}
	//
	//AddBox(g_noBounceMaterial, 10, vec3(2, 2, 2), vec3(2, 20, 0));

	//AddBox(g_noBounceMaterial, 10, vec3(100, 100, 100), vec3(0, 100, 0));

	spawnTimer = 0.0f;
	shootTimer = 0.0f;

	m_gun = m_renderer->LoadOBJ("../data/gun/crossbow.obj");
	m_renderer->LoadTexture("../data/gun/texture.jpg", m_gun);
	m_renderer->LoadAmbient("../data/gun/texture.jpg", m_gun);
	m_renderer->LoadSpecularMap("../data/gun/specular.jpg", m_gun);

	m_playerLight = m_renderer->CreatePointLight(vec3(0.3f, 0.3f, 0.3f), 4.0f, false);

	TwAddSeparator(m_debugBar, "Lights", "");
	TwAddButton(m_debugBar, "AddLight", AddLight, (void*)(m_renderer), "");

	return 0;
}

void Physics2::Update(float a_deltaTime)
{
	UpdatePhysX(a_deltaTime);

	spawnTimer += a_deltaTime;		
	shootTimer -= a_deltaTime;

	if (spawnTimer >= 1.00f)
	{
		std::cout << m_models.size() << std::endl;
		if (rand() % 2 == 0)
			AddBox(g_capsuleMaterial, 10.0f, vec3(2.0f, 2.0f, 2.0f), vec3(((float)rand() / (float)RAND_MAX) * 20.0f, ((float)rand() / (float)RAND_MAX) * 20.0f + 20.0f, ((float)rand() / (float)RAND_MAX) * 20.0f));
		else
			AddSphere(g_sphereMaterial, 10.0f, 2.0f, vec3(((float)rand() / (float)RAND_MAX) * 20.0f, ((float)rand() / (float)RAND_MAX) * 20.0f + 20.0f, ((float)rand() / (float)RAND_MAX) * 20.0f));

		spawnTimer = 0;
	}

	m_camera->Update(a_deltaTime);

	//Shooting and moving the gun should happen after updating the camera.
	m_renderer->SetTransform(glm::translate(glm::scale(glm::rotate(m_camera->GetWorldTransform(), 0.2f, vec3(1, 0, 0)), vec3(0.005f, 0.005f, 0.005f)), vec3(0.0f, -25.0f, -5.0f)), m_gun);
	if (shootTimer > 0.0f)
	{
		//Move the gun back if reloading.
		m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_gun), vec3(0, 0, 5 * shootTimer)), m_gun);
	}

	m_renderer->SetLightPosition(m_playerLight, vec3(m_camera->GetWorldTransform()[3]));
	if (InputManager::GetMouseDown(0) && shootTimer < 0.0f)
	{
		shootTimer = 0.5f;
		AddSphere(g_sphereMaterial, 10.0f, 2.0f, vec3(m_camera->GetWorldTransform()[3]));
		vec3 forward = glm::rotateY(vec3(m_camera->GetWorldTransform()[2]), -0.2f);
		((PxRigidDynamic*)(g_physicsActors[g_physicsActors.size() - 1]))->setLinearVelocity(PxVec3(forward.x, forward.y, forward.z) * -100.0f);
	}
}

void Physics2::Draw()
{
	m_renderer->Draw();
}

int Physics2::Deinit()
{
	delete m_camera;

	m_renderer->CleanupBuffers();

	g_physicsScene->release();
	g_physics->release();
	g_physicsFoundation->release();

	return Application::Deinit();
}

void Physics2::SetUpPhysX()
{
	PxAllocatorCallback *myCallback = new myAllocator();
	g_physicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, g_defaultErrorCallback);
	g_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_physicsFoundation, PxTolerancesScale());
	PxInitExtensions(*g_physics);
	g_physicsMaterial = g_physics->createMaterial(0.5f, 0.5f, 0.5f);
	g_boxMaterial = g_physics->createMaterial(0.1f, 0.1f, 0.9f);
	g_capsuleMaterial = g_physics->createMaterial(0.5f, 0.5f, 0.5f);
	g_sphereMaterial = g_physics->createMaterial(0.9f, 0.9f, 0.1f);
	g_noBounceMaterial = g_physics->createMaterial(0.9f, 0.9f, 0.0f);
	PxSceneDesc sceneDesc(g_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -10.0f, 0);
	sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	g_physicsScene = g_physics->createScene(sceneDesc);

	SetUpVisualDebugger();
}

void Physics2::SetUpVisualDebugger()
{
	//Check if the pbdconnection manager is available on this platform.
	if (g_physics->getPvdConnectionManager() == NULL)
	{
		std::cout << "Error: PhysX Visual Debugger not working." << std::endl;
		return;
	}
	//Setup connection parameters
	//IP of the PC which is running PVD
	const char* pvdHostIP = "127.0.0.1";
	//TCP port to connect to, where PVD is listening
	int port = 5425;
	//Timeout in milliseconds to wait for PVD to respond, consoles and remote PC's need a higher timeout.
	unsigned int timeout = 100;

	PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();
	//And now try to connect.
	auto connection = PxVisualDebuggerExt::createConnection(g_physics->getPvdConnectionManager(), pvdHostIP, port, timeout, connectionFlags);
}

void Physics2::UpdatePhysX(float a_deltaTime)
{
	if (a_deltaTime <= 0)
	{
		return;
	}
	g_physicsScene->simulate(a_deltaTime);
	while (g_physicsScene->fetchResults() == false)
	{
		//Represent all physics objects in the scene.
		for (unsigned int i = 0; i < g_physicsActors.size(); ++i)
		{
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
			
			m_renderer->SetTransform(glm::scale(transform, vec3(m_scales[i])), m_models[i]);

			delete[] shapes;
		}

	}
}

void Physics2::AddBox(PxMaterial* a_material, float a_density, vec3 a_dimensions, vec3 a_position)
{
	PxBoxGeometry box(a_dimensions.x, a_dimensions.y, a_dimensions.z);
	PxTransform position(PxVec3(a_position.x, a_position.y, a_position.z));
	PxCreateDynamic(*g_physics, position, box, *a_material, a_density);
	g_physicsActors.push_back(PxCreateDynamic(*g_physics, position, box, *a_material, a_density));
	g_physicsScene->addActor(*g_physicsActors[g_physicsActors.size() - 1]);

	unsigned int cube = m_renderer->LoadOBJ("../data/cube.obj");
	m_renderer->LoadTexture("../data/crate.png", cube);
	m_renderer->LoadAmbient("../data/crate.png", cube);
	m_models.push_back(cube);
	m_scales.push_back(a_dimensions);
	m_renderer->SetTransform(glm::translate(a_position) * glm::scale(a_dimensions), cube);
}

void Physics2::AddSphere(PxMaterial* a_material, float a_density, float a_radius, vec3 a_position)
{
	PxSphereGeometry sphere(a_radius);
	PxTransform position(PxVec3(a_position.x, a_position.y, a_position.z));
	PxCreateDynamic(*g_physics, position, sphere, *a_material, a_density);
	g_physicsActors.push_back(PxCreateDynamic(*g_physics, position, sphere, *a_material, a_density));
	g_physicsScene->addActor(*g_physicsActors[g_physicsActors.size() - 1]);

	unsigned int renderSphere = m_renderer->LoadOBJ("../data/sphere/sphere.obj");
	m_renderer->LoadTexture("../data/crate.png", renderSphere);
	m_renderer->LoadAmbient("../data/crate.png", renderSphere);
	m_models.push_back(renderSphere);
	m_scales.push_back(vec3(a_radius, a_radius, a_radius));
	m_renderer->SetTransform(glm::translate(a_position) * glm::scale(vec3(a_radius, a_radius, a_radius)), renderSphere);
}