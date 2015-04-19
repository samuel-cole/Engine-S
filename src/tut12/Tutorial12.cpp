#include "Tutorial12.h"
#include "glm\ext.hpp"
#include "StaticCamera.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "AntTweakBar.h"

void TW_CALL Regenerate(void* a_clientData)
{
	ButtonInfo* buttonInfo = (ButtonInfo*)a_clientData;
	buttonInfo->renderer->GeneratePerlinNoiseMap(100, 100, 6, buttonInfo->amplitude, buttonInfo->persistence, buttonInfo->object, buttonInfo->seed);
}

int Tutorial12::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_buttonInfo.renderer = new Renderer(m_camera, m_debugBar);


	//std::vector<std::string> textures;
	//std::vector<std::string> normalMaps;
	//std::vector<std::string> specularMaps;
	//std::vector<bool> texChannels;
	//std::vector<bool> normChannels;
	//std::vector<bool> specularChannels;
	//
	//textures.push_back("../data/crate.png");
	//normalMaps.push_back("../data/Enemyelite/EnemyElite_N.tga");
	//specularMaps.push_back("../data/Enemyelite/EnemyElite_S.tga");
	//
	//m_renderer->LoadFBX("../data/stanford/cube.fbx", &textures, &normalMaps, &specularMaps);
	//
	//m_renderer->GeneratePerlinNoiseMap(100, 100, 6, 0.1, 0.3f, 1);
	

	//Ground
	//m_buttonInfo.amplitude = 20.0f;
	//m_buttonInfo.persistence = 0.3f;
	//m_buttonInfo.seed = 0;
	//m_buttonInfo.object = m_buttonInfo.renderer->GenerateGrid(100, 100);
	//m_buttonInfo.renderer->LoadTexture("../data/crate.png", m_buttonInfo.object);
	//m_buttonInfo.renderer->GeneratePerlinNoiseMap(100, 100, 6, m_buttonInfo.amplitude, m_buttonInfo.persistence, m_buttonInfo.object, m_buttonInfo.seed);

	//Planet
	m_buttonInfo.amplitude = 0.01f;
	m_buttonInfo.persistence = 1.5f;
	m_buttonInfo.seed = 0;
	m_buttonInfo.object = m_buttonInfo.renderer->LoadOBJ("../data/sphere/sphere.obj");
	m_buttonInfo.renderer->LoadTexture("../data/vanquish/upper_d.tga", m_buttonInfo.object);
	m_buttonInfo.renderer->LoadNormalMap("../data/vanquish/upper_n.tga", m_buttonInfo.object);
	m_buttonInfo.renderer->GeneratePerlinNoiseMap(100, 100, 6, m_buttonInfo.amplitude, m_buttonInfo.persistence, m_buttonInfo.object, m_buttonInfo.seed);
	
	//Lower suit
	//location = m_renderer->LoadOBJ("../data/vanquish/lower.obj");
	//m_renderer->LoadTexture("../data/vanquish/lower_d.tga", location);
	//m_renderer->LoadNormalMap("../data/vanquish/lower_n.tga", location);
	////m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	//
	////Upper suit
	//location = m_renderer->LoadOBJ("../data/vanquish/upper.obj");
	//m_renderer->LoadTexture("../data/vanquish/upper_d.tga", location);
	//m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", location);
	////m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	//
	////Inner suit
	//location = m_renderer->LoadOBJ("../data/vanquish/inner.obj");
	//m_renderer->LoadTexture("../data/vanquish/inner_d.tga", location);
	//m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", location);
	////m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	//
	////Hand
	//location = m_renderer->LoadOBJ("../data/vanquish/hand.obj");
	//m_renderer->LoadTexture("../data/vanquish/inner_d.tga", location);
	//m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", location);
	////m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	//
	////Buttons
	//location = m_renderer->LoadOBJ("../data/vanquish/buttons.obj");
	//m_renderer->LoadTexture("../data/vanquish/upper_d.tga", location);
	//m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", location);
	////m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	//
	////Visor
	//location = m_renderer->LoadOBJ("../data/vanquish/visor.obj");
	//m_renderer->LoadTexture("../data/vanquish/visor.tga", location);

	TwAddSeparator(m_debugBar, "Procedural Generation", "");
	TwAddVarRW(m_debugBar, "Amplitude", TW_TYPE_FLOAT, &m_buttonInfo.amplitude, "");
	TwAddVarRW(m_debugBar, "Persistence", TW_TYPE_FLOAT, &m_buttonInfo.persistence, "");
	TwAddVarRW(m_debugBar, "Seed", TW_TYPE_UINT32, &m_buttonInfo.seed, "");
	TwAddButton(m_debugBar, "Re-generate", Regenerate, (void*)&m_buttonInfo, "");

	return 0;
}

void Tutorial12::Update(float a_deltaTime)
{
	m_camera->Update(a_deltaTime);
}

void Tutorial12::Draw()
{
	m_buttonInfo.renderer->Draw();
}

int Tutorial12::Deinit()
{
	delete m_camera;

	m_buttonInfo.renderer->CleanupBuffers();

	return Application::Deinit();
}