#include "PhysicsBase.h"
#include "glm\ext.hpp"
#include "WalkCamera.h"
#include "Renderer.h"
#include "InputManager.h"

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

int PhysicsBase::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new WalkCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));
	
	m_renderer = new Renderer(m_camera, m_debugBar);

	SetUpPhysX();

	return 0;
}

void PhysicsBase::Update(float a_deltaTime)
{
	UpdatePhysX(a_deltaTime);

	m_camera->Update(a_deltaTime);
}

void PhysicsBase::Draw()
{
	m_renderer->Draw();
}

int PhysicsBase::Deinit()
{
	m_renderer->CleanupBuffers();
	delete m_renderer;

	delete m_camera;

	for (unsigned int i = 0; i < g_physicsActors.size(); ++i)
	{
		if (g_physicsActors[i] != nullptr)
		{
			PxU32 shapeNo = g_physicsActors[i]->getNbShapes();
			PxShape** shapes = new PxShape*[shapeNo];
			g_physicsActors[i]->getShapes(shapes, shapeNo);
			for (unsigned int j = 0; j < shapeNo; ++j)
			{
				g_physicsActors[i]->detachShape(*shapes[j]);
			}
			delete[] shapes;

			g_physicsActors[i]->release();
			g_physicsActors[i] = nullptr;
		}
		g_physicsActors.clear();
	}
	for (unsigned int i = 0; i < g_physicsCloths.size(); ++i)
	{
		if (g_physicsCloths[i] != nullptr)
		{
			g_physicsCloths[i]->getFabric()->release();
			g_physicsCloths[i]->release();
			g_physicsCloths[i] = nullptr;
		}
	}
	if (g_terrain != nullptr)
	{
		unsigned int shapeNo = g_terrain->getNbShapes();
		PxShape** shapes = new PxShape*[shapeNo];
		g_terrain->getShapes(shapes, 1);
		g_terrain->detachShape(**shapes);
		g_terrain->release();
		g_terrain = nullptr;
		delete[] shapes;
	}

	if (g_cpuDispatcher != nullptr)
	{
		//_aligned_free(g_cpuDispatcher);
		delete g_cpuDispatcher;
		g_cpuDispatcher = nullptr;
	}

#if _DEBUG
	if (g_pvdConnection != nullptr)
	{
		g_pvdConnection->disconnect();
		g_pvdConnection->release();
		g_pvdConnection = nullptr;
	}
#endif

	if (g_physicsScene != nullptr)
	{
		g_physicsScene->release();
		g_physicsScene = nullptr;
	}
	if (g_physics != nullptr)
	{
		PxCloseExtensions();
		g_physics->release();
		g_physics = nullptr;
	}
	if (g_physicsFoundation != nullptr)
	{
		//TODO: Fix this line. It currently doesn't release the foundation 'due to pending module references.' 'Close/release all depending modules first.'
		g_physicsFoundation->release();
		g_physicsFoundation = nullptr;
	}
	if (g_allocatorCallback != nullptr)
	{
		delete g_allocatorCallback;
		g_allocatorCallback = nullptr;
	}

	return Application::Deinit();
}

void PhysicsBase::SetUpPhysX()
{
	g_allocatorCallback = new myAllocator();
	g_physicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *g_allocatorCallback, g_defaultErrorCallback);
	g_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_physicsFoundation, PxTolerancesScale());
	PxInitExtensions(*g_physics);
	PxSceneDesc sceneDesc(g_physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -10.0f, 0);
	sceneDesc.filterShader = &PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	g_cpuDispatcher = sceneDesc.cpuDispatcher;

	g_physicsScene = g_physics->createScene(sceneDesc);

	g_terrain = nullptr;

#if _DEBUG
	SetUpVisualDebugger();
#endif
}

void PhysicsBase::SetUpVisualDebugger()
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
	g_pvdConnection = PxVisualDebuggerExt::createConnection(g_physics->getPvdConnectionManager(), pvdHostIP, port, timeout, connectionFlags);
}

void PhysicsBase::UpdatePhysX(float a_deltaTime)
{
	if (a_deltaTime <= 0)
	{
		return;
	}
	g_physicsScene->simulate(a_deltaTime);
	while (g_physicsScene->fetchResults() == false)
	{
		//Represent all normal physics objects in the scene.
		for (unsigned int i = 0; i < g_physicsActors.size(); ++i)
		{
			if (m_models[i] == -1)
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

			m_renderer->SetTransform(glm::scale(transform, vec3(m_scales[i])), m_models[i]);

			delete[] shapes;
		}
	}

	//Represent cloth objects within the scene.
	for (unsigned int i = 0; i < g_physicsCloths.size(); ++i)
	{
		PxClothParticleData* data = g_physicsCloths[i]->lockParticleData();
		std::vector<vec3> particlePositions;
		for (unsigned int j = 0; j < g_physicsCloths[i]->getNbParticles(); ++j)
		{
			PxVec3 particlePos = data->particles[j].pos;
			particlePositions.push_back(vec3(particlePos.x, particlePos.y, particlePos.z));
		}
		m_renderer->ModifyMesh(m_clothModels[i], particlePositions);
		data->unlock();
	}
}

void PhysicsBase::AddBox(PxMaterial* const a_material, const float a_density, const vec3& a_dimensions, const vec3& a_position, const bool a_visible)
{
	PxBoxGeometry box(a_dimensions.x, a_dimensions.y, a_dimensions.z);
	PxTransform position(PxVec3(a_position.x, a_position.y, a_position.z));
	g_physicsActors.push_back(PxCreateDynamic(*g_physics, position, box, *a_material, a_density));
	g_physicsScene->addActor(*g_physicsActors[g_physicsActors.size() - 1]);

	if (a_visible)
	{
		unsigned int cube = m_renderer->LoadOBJ("../data/cube.obj");
		m_renderer->LoadTexture("../data/crate.png", cube);
		m_renderer->LoadAmbient("../data/crate.png", cube);
		m_models.push_back(cube);
		m_scales.push_back(a_dimensions);
		m_renderer->SetTransform(glm::translate(a_position) * glm::scale(a_dimensions), cube);
	}
	else
	{
		m_models.push_back(-1);
		m_scales.push_back(vec3(-1, -1, -1));
	}
}

void PhysicsBase::AddSphere(PxMaterial* const a_material, const float a_density, const float a_radius, const vec3& a_position, const bool a_visible)
{
	PxSphereGeometry sphere(a_radius);
	PxTransform position(PxVec3(a_position.x, a_position.y, a_position.z));
	PxCreateDynamic(*g_physics, position, sphere, *a_material, a_density);
	g_physicsActors.push_back(PxCreateDynamic(*g_physics, position, sphere, *a_material, a_density));
	g_physicsScene->addActor(*g_physicsActors[g_physicsActors.size() - 1]);

	if (a_visible)
	{
		unsigned int renderSphere = m_renderer->LoadOBJ("../data/sphere/sphere.obj");
		m_renderer->LoadTexture("../data/crate.png", renderSphere);
		m_renderer->LoadAmbient("../data/crate.png", renderSphere);
		m_models.push_back(renderSphere);
		m_scales.push_back(vec3(a_radius, a_radius, a_radius));
		m_renderer->SetTransform(glm::translate(a_position) * glm::scale(vec3(a_radius, a_radius, a_radius)), renderSphere);
	}
	else
	{
		m_models.push_back(-1);
		m_scales.push_back(vec3(-1, -1, -1));
	}
}

PxRigidStatic* PhysicsBase::AddProceduralPlane(const unsigned int a_dimensions, const unsigned int a_noiseMapDimensions,
											   const float a_stretch, const vec3& a_position, PxMaterial* const a_material, unsigned int& a_rendererIndex, float& a_maxHeight,
											   const float a_amplitude, const unsigned int a_seed, const unsigned int a_octaves, const float a_persistence)
{
	if (g_terrain != nullptr)
	{
		g_terrain->release();
		g_terrain = nullptr;
	}

	std::vector<float> proceduralHeights;

	a_rendererIndex = m_renderer->GenerateGrid(a_dimensions, a_dimensions);
	m_renderer->GeneratePerlinNoiseMap(a_noiseMapDimensions, a_noiseMapDimensions, a_octaves, a_amplitude, a_persistence, a_rendererIndex, a_seed, false, true, proceduralHeights);
	m_renderer->SetTransform(glm::scale(glm::translate(a_position), vec3(a_stretch, 1, a_stretch)), a_rendererIndex);

	//This is used for getting the maximum possible precision from PhysX for my heightmap.
	//This is the highest number that I can use.
	float maxNumber = (1 << (sizeof(PxI16)* 8)) / 2 - 1;

	a_maxHeight = -9999999.9f;
	for (unsigned int i = 0; i < proceduralHeights.size(); ++i)
	{
		if (proceduralHeights[i] > a_maxHeight)
		{
			a_maxHeight = proceduralHeights[i];
		}
	}

	if (a_maxHeight == -9999999.9f)
	{
		std::cout << "Error: Sending invalid perlin heights to PhysX." << std::endl;
		return nullptr;
	}
	else
	{
		//PhysX uses a short to store heights, so I use this to find the largest possible number that I can multiply by without losing wrapping around.
		float multiplier = (maxNumber / a_maxHeight);


		//I could do row stuff with maths, but this makes the logic a lot more human-readable.
		unsigned int row = 0;
		//+1 to get from a_dimensions into the intuitive definition of dimensions (human readability), +2 more to add padding.
		unsigned int paddedDimensions = a_dimensions + 3;
		//Get the heights of the procedural terrain into a form accepted by PhysX. Extra spacing added for outside row.
		PxHeightFieldSample* data = new PxHeightFieldSample[proceduralHeights.size()];
		unsigned int counter = 0;

		for (unsigned int i = a_dimensions; i < proceduralHeights.size(); --i)
		{
			data[counter].height = (short)(proceduralHeights[i] * multiplier);

			if (i % (a_dimensions + 1) == 0)
			{
				i += (a_dimensions + 1) * 2;
			}
			++counter;
		}

		//Generate the height
		PxHeightFieldDesc heightFieldDesc;
		heightFieldDesc.format = PxHeightFieldFormat::eS16_TM;
		heightFieldDesc.nbColumns = a_dimensions + 1;
		heightFieldDesc.nbRows = a_dimensions + 1;
		heightFieldDesc.samples.data = data;
		heightFieldDesc.samples.stride = sizeof(PxHeightFieldSample);
		heightFieldDesc.thickness = -100.0f;

		//Make the physics representation of the heightfield.
		PxHeightField* heightField = g_physics->createHeightField(heightFieldDesc);
		PxHeightFieldGeometry heightFieldGeometry(heightField, PxMeshGeometryFlags(), 1.0f / multiplier, a_stretch, a_stretch);
		float translate = paddedDimensions * a_stretch / 2.0f;
		PxTransform pose = PxTransform(PxVec3(translate, 0.0f, -translate) + PxVec3(a_position.x, a_position.y, a_position.z), PxQuat(PxHalfPi, PxVec3(0, -1, 0)));

		PxShape* heightFieldShape = g_physics->createShape(heightFieldGeometry, *a_material);
		PxRigidStatic* terrain = PxCreateStatic(*g_physics, pose, *heightFieldShape);
		g_physicsScene->addActor(*terrain);
		g_terrain = terrain;

		delete[] data;

		return terrain;
	}	
}

void PhysicsBase::AddCloth(unsigned int a_dimensions, std::vector<unsigned int> a_staticPoints, PxTransform a_pose)
{
	//Fill in the data for how many points there should be, where they should be, etc.
	PxClothParticle* vertices = new PxClothParticle[a_dimensions * a_dimensions];
	for (unsigned int r = 0; r < a_dimensions; ++r)
	{
		for (unsigned int c = 0; c < a_dimensions; ++c)
		{
			vertices[r * a_dimensions + c].pos = PxVec3((float)c - a_dimensions / 2, 0, (float)r - a_dimensions / 2);
			vertices[r * a_dimensions + c].invWeight = 1.0f;
		}
	}
	//Make the appropriate points static.
	for (unsigned int i = 0; i < a_staticPoints.size(); ++i)
	{
		if (a_staticPoints[i] < a_dimensions * a_dimensions)
			vertices[a_staticPoints[i]].invWeight = 0.0f;
		else
			std::cout << "Error: invalid point index while creating cloth.";
	}
	
	//The quads field that this primitives variable is used for is essentially the physX equivalent of OpenGL indices (IBOs).
	PxU32* primitives = new PxU32[(a_dimensions - 1) * (a_dimensions - 1) * 4];
	unsigned int index = 0;
	for (unsigned int r = 0; r < (a_dimensions - 1); ++r)
	{
		for (unsigned int c = 0; c < (a_dimensions - 1); ++c)
		{
			primitives[index++] = r * a_dimensions + c;
			primitives[index++] = r * a_dimensions + c + 1;
			primitives[index++] = (r + 1) * a_dimensions + c;
			primitives[index++] = (r + 1) * a_dimensions + c + 1;
		}
	}
	
	//Fill in the description object.
	PxClothMeshDesc clothDesc;
	clothDesc.points.data = vertices;
	clothDesc.points.count = a_dimensions * a_dimensions;
	clothDesc.points.stride = sizeof(PxClothParticle);
	clothDesc.invMasses.data = &vertices->invWeight;
	clothDesc.invMasses.count = a_dimensions * a_dimensions;
	clothDesc.invMasses.stride = sizeof(PxClothParticle);
	clothDesc.quads.data = primitives;
	clothDesc.quads.count = (a_dimensions - 1) * (a_dimensions - 1);
	clothDesc.quads.stride = sizeof(PxU32) * 4;
	
	//Create the cloth.
	PxClothFabric* fabric = PxClothFabricCreate(*g_physics, clothDesc, PxVec3(0, -1, 0));
	PxCloth* cloth = g_physics->createCloth(a_pose, *fabric, vertices, PxClothFlags());
	cloth->setClothFlag(PxClothFlag::eSCENE_COLLISION, true);
	cloth->setSolverFrequency(240.0f);
	g_physicsScene->addActor(*cloth);
	g_physicsCloths.push_back(cloth);
	
	delete[] vertices;
	delete[] primitives;

	m_clothModels.push_back(m_renderer->GenerateGrid(a_dimensions - 1, a_dimensions - 1));
	glm::quat rot = glm::quat(a_pose.q.w, a_pose.q.x, a_pose.q.y, a_pose.q.z);
	m_renderer->SetTransform((mat4)rot * glm::translate(vec3(a_pose.p.x, a_pose.p.y, a_pose.p.z)), m_clothModels[m_clothModels.size() - 1]);
}
