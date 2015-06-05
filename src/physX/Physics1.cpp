#include "Physics1.h"
#include "glm\ext.hpp"
#include "FlyCamera.h"
#include "Renderer.h"

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

int Physics1::Init()
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
	PxTransform pose = PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(PxHalfPi * 0.5f, PxVec3(0.0f, 0.0f, 1.0f)));
	PxRigidStatic* plane = PxCreateStatic(*g_physics, pose, PxPlaneGeometry(), *g_physicsMaterial);
	g_physicsScene->addActor(*plane);

	//Add a box
	float density = 10;
	PxBoxGeometry box(2, 2, 2);
	PxTransform transform(PxVec3(0, 5, 0));
	PxRigidDynamic* dynamicActor = PxCreateDynamic(*g_physics, transform, box, *g_physicsMaterial, density);

	//Add it to the physX scene
	g_physicsScene->addActor(*dynamicActor);

	spawnTimer = 0.0f;

	return 0;
}

void Physics1::Update(float a_deltaTime)
{
	UpdatePhysX(a_deltaTime);

	spawnTimer += a_deltaTime;

	if (spawnTimer > 1.0f)
	{
		spawnTimer = 0.0f;

		PxRigidDynamic* dynamicActor;

		if (rand() % 2 == 0)
		{
			//Add a box
			float density = 10;
			PxBoxGeometry box(2, 2, 2);
			PxTransform transform(PxVec3(((float)rand() / (float)RAND_MAX) * 20, ((float)rand() / (float)RAND_MAX) * 20, ((float)rand() / (float)RAND_MAX) * 20));
			dynamicActor = PxCreateDynamic(*g_physics, transform, box, *g_boxMaterial, density);
		}
		else if (rand() % 2 == 0)
		{
			float density = 100;
			PxCapsuleGeometry capsule(2, 2);
			PxTransform transform(PxVec3(((float)rand() / (float)RAND_MAX) * 20, ((float)rand() / (float)RAND_MAX) * 20, ((float)rand() / (float)RAND_MAX) * 20));
			dynamicActor = PxCreateDynamic(*g_physics, transform, capsule, *g_capsuleMaterial, density);
		}
		else
		{
			float density = 100;
			PxSphereGeometry sphere(2);
			PxTransform transform(PxVec3(((float)rand() / (float)RAND_MAX) * 20, ((float)rand() / (float)RAND_MAX) * 20, ((float)rand() / (float)RAND_MAX) * 20));
			dynamicActor = PxCreateDynamic(*g_physics, transform, sphere, *g_sphereMaterial, density);
		}


		//Add it to the physX scene
		g_physicsScene->addActor(*dynamicActor);
	}
}

void Physics1::Draw()
{
	m_renderer->Draw();
}

int Physics1::Deinit()
{
	delete m_camera;

	m_renderer->CleanupBuffers();

	g_physicsScene->release();
	g_physics->release();
	g_physicsFoundation->release();

	return Application::Deinit();
}

void Physics1::SetUpPhysX()
{
	PxAllocatorCallback *myCallback = new myAllocator();
	g_physicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, g_defaultErrorCallback);
	g_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_physicsFoundation, PxTolerancesScale());
	PxInitExtensions(*g_physics);
	g_physicsMaterial = g_physics->createMaterial(0.5f, 0.5f, 0.5f);
	g_boxMaterial = g_physics->createMaterial(0.1f, 0.1f, 10.0f);
	g_capsuleMaterial = g_physics->createMaterial(0.5f, 0.5f, 0.5f);
	g_sphereMaterial = g_physics->createMaterial(0.9f, 0.9f, 0.1f);
	PxSceneDesc sceneDesc(g_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -10.0f, 0);
	sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	g_physicsScene = g_physics->createScene(sceneDesc);

	SetUpVisualDebugger();
}

void Physics1::SetUpVisualDebugger()
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

void Physics1::UpdatePhysX(float a_deltaTime)
{
	if (a_deltaTime <= 0)
	{
		return;
	}
	g_physicsScene->simulate(a_deltaTime);
	while (g_physicsScene->fetchResults() == false)
	{
		//Fill in later.
	}
}