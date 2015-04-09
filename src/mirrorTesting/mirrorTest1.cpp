#include "MirrorTest1.h"
#include "glm\ext.hpp"
#include "StaticCamera.h"
#include "FlyCamera.h"
#include "Renderer.h"


int MirrorTest1::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);

	//Mirror stuff
	//Make reflection camera.
	m_staticCamera = new StaticCamera();
	m_staticCamera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_staticCamera->SetLookAt(vec3(0, 0, 10), vec3(0, 5, 0), vec3(0, 1, 0));
	//Make frame texture.
	unsigned int frameTexture, frameBuffer;
	frameTexture = m_renderer->LoadFrameBuffer(m_staticCamera, vec4(0.0f, 0.0f, 512.0f, 512.0f), vec3(1.0f, 1.0f, 1.0f), frameBuffer);
	//Make mirror
	m_mirror = m_renderer->GenerateGrid(10, 10);
	m_renderer->LoadTexture(frameTexture, m_mirror);
	//Add mirror to ignore list for mirror's framebuffer.
	m_renderer->AddFrameBufferIgnore(frameBuffer, m_mirror);
	//Set mirror position
	m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_mirror), vec3(0, 5, -10)), m_mirror);
	m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), 3.14159265358979f / 2.0f, vec3(1.0f, 0, 0)), m_mirror);
	m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), 3.14159265358979f / 2.0f, vec3(0, 1.0f, 0)), m_mirror);


	std::vector<std::string> textures;
	std::vector<std::string> normalMaps;
	std::vector<std::string> specularMaps;
	std::vector<bool> texChannels;
	std::vector<bool> normChannels;
	std::vector<bool> specularChannels;
	
	textures.push_back("../data/Enemyelite/EnemyElite3_D.tga");
	textures.push_back("../data/Enemyelite/Alienrifle_D.png");
	normalMaps.push_back("../data/Enemyelite/EnemyElite_N.tga");
	normalMaps.push_back("../data/Enemyelite/Alienrifle_N.png");
	specularMaps.push_back("../data/Enemyelite/EnemyElite_S.tga");
	specularMaps.push_back("../data/Enemyelite/Alienrifle_S.tga");
	
	m_model = m_renderer->LoadFBX("../data/Enemyelite/EnemyElite.fbx", &textures, &normalMaps, &specularMaps);

	m_renderer->GenerateShadowMap(50.0f);

	m_timer = 0;

	return 0;
}

void MirrorTest1::Update(float a_deltaTime)
{
	m_timer += a_deltaTime;

	//I think I should be doing this reflection stuff on the shader, so that the reflection for each point can be calculated.
	vec3 cameraPosition = vec3(m_camera->GetWorldTransform()[3]);
	vec3 mirrorPosition = vec3(m_renderer->GetTransform(m_mirror)[3]);
	vec3 incident = glm::normalize(mirrorPosition - cameraPosition);
	vec3 normal = glm::normalize(vec3(m_renderer->GetTransform(m_mirror) * vec4(0, 1, 0, 0)));
	//The reflected vector is equal to Incident - 2 * (Incident.Normal) * Normal, http://www.cosinekitty.com/raytrace/chapter10_reflection.html has a good explanation of how this is derived.
	vec3 reflected = glm::normalize(incident - 2 * glm::dot(incident, normal) * normal);

	//Should the up vector be world up? Should it be the mirror's up? Not sure.
	//m_staticCamera->SetLookAt(mirrorPosition, mirrorPosition + reflected, vec3(0, 1, 0));

	//Instead of doing the above calculations, I am thinking about reflecting the camera through the plane created by the mirror.
	//To reflect a point through a plane:
	//1) Find the closest point on the plane to the point.
	//2) Find the vector from the point to the closest point.
	//3) Add that vector to the closest point position vector.
	//Should I still look at the same point? Not sure.
	//Maybe use this link http://stackoverflow.com/questions/9605556/how-to-project-a-3d-point-to-a-3d-plane

	vec3 v = cameraPosition - mirrorPosition;
	float dist = glm::dot(v, normal);
	//closestPoint = cameraPosition - dist * normal, therefore:
	vec3 reflectedPoint = cameraPosition - 2 * (dist*normal);


	m_staticCamera->Update(a_deltaTime);

	m_staticCamera->SetLookAt(reflectedPoint, mirrorPosition, vec3(0, 1, 0));
	
	m_camera->Update(a_deltaTime);
	//m_renderer->UpdateAnimation(m_timer, m_model);

	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), a_deltaTime, vec3(1.0f, 0, 0)), m_mirror);
	//m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(3), vec3(a_deltaTime, 0, 0)), 3);
	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_model), a_deltaTime, vec3(0, 1, 0)), m_model);
}

void MirrorTest1::Draw()
{
	m_renderer->Draw();
}

int MirrorTest1::Deinit()
{
	delete m_camera;
	delete m_staticCamera;
	
	m_renderer->CleanupBuffers();

	return Application::Deinit();
}