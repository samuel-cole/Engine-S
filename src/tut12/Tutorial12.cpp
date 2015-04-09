#include "Tutorial12.h"
#include "glm\ext.hpp"
#include "StaticCamera.h"
#include "FlyCamera.h"
#include "Renderer.h"


int Tutorial12::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);


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
	unsigned int location = m_renderer->GenerateGrid(100, 100);
	m_renderer->LoadTexture("../data/crate.png", location);
	m_renderer->GeneratePerlinNoiseMap(100, 100, 6, 20.0f, 0.3f, location);
	//
	//Lower suit
	location = m_renderer->LoadOBJ("../data/vanquish/lower.obj");
	m_renderer->LoadTexture("../data/vanquish/lower_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/lower_n.tga", location);
	//m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	
	//Upper suit
	location = m_renderer->LoadOBJ("../data/vanquish/upper.obj");
	m_renderer->LoadTexture("../data/vanquish/upper_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", location);
	//m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	
	//Inner suit
	location = m_renderer->LoadOBJ("../data/vanquish/inner.obj");
	m_renderer->LoadTexture("../data/vanquish/inner_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", location);
	//m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	
	//Hand
	location = m_renderer->LoadOBJ("../data/vanquish/hand.obj");
	m_renderer->LoadTexture("../data/vanquish/inner_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", location);
	//m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	
	//Buttons
	location = m_renderer->LoadOBJ("../data/vanquish/buttons.obj");
	m_renderer->LoadTexture("../data/vanquish/upper_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", location);
	//m_renderer->LoadSpecularMap("../data/Enemyelite/EnemyElite_S.tga", location);
	
	//Visor
	location = m_renderer->LoadOBJ("../data/vanquish/visor.obj");
	m_renderer->LoadTexture("../data/vanquish/visor.tga", location);

	return 0;
}

void Tutorial12::Update(float a_deltaTime)
{
	m_camera->Update(a_deltaTime);
}

void Tutorial12::Draw()
{
	m_renderer->Draw();
}

int Tutorial12::Deinit()
{
	delete m_camera;

	m_renderer->CleanupBuffers();

	return Application::Deinit();
}