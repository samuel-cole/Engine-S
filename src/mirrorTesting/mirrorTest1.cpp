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

	std::vector<unsigned int> frameTextures;

	m_staticCamera.push_back(new StaticCamera());
	m_staticCamera[0]->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_staticCamera[0]->SetLookAt(vec3(0, 0, 10), vec3(0, 5, 0), vec3(0, 1, 0));

	frameTextures.push_back(-1);
	frameTextures[0] = m_renderer->LoadFrameBuffer(m_staticCamera[0], vec4(0.0f, 0.0f, 512.0f, 512.0f), vec3(1.0f, 1.0f, 1.0f));

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

	unsigned int grid = m_renderer->GenerateGrid(10, 10, glm::vec3(0, 0, 0));
	m_renderer->LoadTexture(frameTextures[0], grid);

	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(grid), 3.14159265358979f/2.0f, vec3(1.0f, 0, 0)), grid);
	//m_renderer->SetTransform(glm::translate(glm::mat4(), vec3(0, 0, 1000.0f)) * m_renderer->GetTransform(grid), grid);
	m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(grid), vec3(100, 0, 0)), grid);

	m_renderer->GenerateShadowMap(50.0f);

	m_timer = 0;

	return 0;
}

void MirrorTest1::Update(float a_deltaTime)
{
	m_timer += a_deltaTime;
	for (unsigned int i = 0; i < m_staticCamera.size(); ++i)
	{
		m_staticCamera[i]->Update(a_deltaTime);
	}
	m_camera->Update(a_deltaTime);
	m_renderer->UpdateAnimation(m_timer, m_model);
}

void MirrorTest1::Draw()
{
	m_renderer->Draw();
}

int MirrorTest1::Deinit()
{
	delete m_camera;
	for (unsigned int i = 0; i < m_staticCamera.size(); ++i)
	{
		delete m_staticCamera[i];
	}


	m_renderer->CleanupBuffers();

	return Application::Deinit();
}