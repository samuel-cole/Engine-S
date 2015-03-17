#include "Tutorial10.h"
#include "glm\ext.hpp"
#include "StaticCamera.h"
#include "FlyCamera.h"
#include "Renderer.h"


int Tutorial10::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);

	m_staticCamera = new StaticCamera();
	m_staticCamera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_staticCamera->SetLookAt(vec3(10, 10, 10), vec3(0, 10, 0), vec3(0, 1, 0));

	unsigned int texture;
	m_frameBuffer = m_renderer->LoadFrameBuffer(m_staticCamera, vec4(0.0f, 0.0f, 512.0f, 512.0f), vec3(1.0f, 1.0f, 1.0f), texture);

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
	texChannels.push_back(false);
	texChannels.push_back(false);
	normChannels.push_back(false);
	normChannels.push_back(false);
	specularChannels.push_back(false);
	specularChannels.push_back(false);

	m_renderer->LoadFBX("../data/Enemyelite/EnemyElite.fbx", &textures, &normalMaps, &specularMaps, &texChannels, &normChannels, &specularChannels);

	unsigned int grid = m_renderer->GenerateGrid(1000, 1000);
	m_renderer->LoadTexture(texture, grid);


	m_timer = 0;

	return 0;
}

void Tutorial10::Update(float a_deltaTime)
{
	m_timer += a_deltaTime;
	m_staticCamera->Update(a_deltaTime);
	m_camera->Update(a_deltaTime);
	m_renderer->UpdateAnimation(m_timer);
}

void Tutorial10::Draw()
{
	m_renderer->Draw();
}

int Tutorial10::Deinit()
{
	delete m_camera;
	delete m_staticCamera;

	m_renderer->CleanupBuffers();

	return Application::Deinit();
}