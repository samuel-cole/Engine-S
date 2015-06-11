#include "Physics2.h"
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

void Physics2::Update(float a_deltaTime)
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
		//Fill in later.
	}
}

void Physics2::AddBox(PxMaterial* a_material, float a_density, vec3 a_dimensions, vec3 a_position)
{
	PxBoxGeometry box(a_dimensions.x, a_dimensions.y, a_dimensions.z);
	PxTransform position(PxVec3(a_position.x, a_position.y, a_position.z));
	PxRigidDynamic *dynamicActor = PxCreateDynamic(*g_physics, position, box, *a_material, a_density);


	unsigned int cube = m_renderer->LoadOBJ("../data/cube.obj");
	m_renderer->LoadTexture("../data/crate.png", cube);
	m_renderer->LoadAmbient("../data/crate.png", cube);
	m_models.push_back(cube);

	PxMat44 m(PxShapeExt::getGlobalPose(*box, *dynamicActor));
	glm::mat4 transform(m.column0.x, m.column0.y, m.column0.z, m.column0.w,
						m.column1.x, m.column1.y, m.column1.z, m.column1.w,
						m.column2.x, m.column2.y, m.column2.z, m.column2.w,
						m.column3.x, m.column3.y, m.column3.z, m.column3.w);	m_renderer->SetTransform(transform, cube);
}