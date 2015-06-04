#include "Tutorial13.h"
#include "glm\glm.hpp"
#include "glm\ext.hpp"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "glm\glm.hpp"

#include <thread>
#include <vector>

void TW_CALL AddLight(void* a_clientData)
{
	Renderer* renderer = (Renderer*)a_clientData;
	renderer->CreatePointLight(vec3(1, 0, 0), 1, true);
}

int Tutorial13::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 1000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));



	auto major = ogl_GetMajorVersion();
	auto minor = ogl_GetMinorVersion();
	printf("GL: %i.%i\n", major, minor);

	m_renderer = new Renderer(m_camera, m_debugBar);

	for (int i = 0; i < 1000; ++i)
	{
		glm::mat4 randomPos = glm::translate(vec3(((float)rand() / (float)RAND_MAX) * 100.0f, ((float)rand() / (float)RAND_MAX) * 100.0f, ((float)rand() / (float)RAND_MAX) * 100.0f));

		//Lower suit
		unsigned int location = m_renderer->LoadOBJ("../data/vanquish/lower.obj");
		m_renderer->LoadAmbient("../data/vanquish/lower_d.tga", location);
		m_renderer->LoadTexture("../data/vanquish/lower_d.tga", location);
		m_renderer->LoadSpecularMap("../data/vanquish/lower_d.tga", location);
		m_renderer->LoadNormalMap("../data/vanquish/lower_n.tga", location);

		m_renderer->SetTransform(randomPos, location);

		//Upper suit
		location = m_renderer->LoadOBJ("../data/vanquish/upper.obj");
		m_renderer->LoadAmbient("../data/vanquish/upper_d.tga", location);
		m_renderer->LoadTexture("../data/vanquish/upper_d.tga", location);
		m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", location);
		m_renderer->LoadSpecularMap("../data/vanquish/upper_d.tga", location);

		m_renderer->SetTransform(randomPos, location);

		//Inner suit
		location = m_renderer->LoadOBJ("../data/vanquish/inner.obj");
		m_renderer->LoadAmbient("../data/vanquish/inner_d.tga", location);
		m_renderer->LoadTexture("../data/vanquish/inner_d.tga", location);
		m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", location);
		m_renderer->LoadSpecularMap("../data/vanquish/inner_d.tga", location);

		m_renderer->SetTransform(randomPos, location);

		//Hand
		location = m_renderer->LoadOBJ("../data/vanquish/hand.obj");
		m_renderer->LoadAmbient("../data/vanquish/inner_d.tga", location);
		m_renderer->LoadTexture("../data/vanquish/inner_d.tga", location);
		m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", location);
		m_renderer->LoadSpecularMap("../data/vanquish/inner_d.tga", location);

		m_renderer->SetTransform(randomPos, location);

		//Buttons
		location = m_renderer->LoadOBJ("../data/vanquish/buttons.obj");
		m_renderer->LoadAmbient("../data/vanquish/upper_d.tga", location);
		m_renderer->LoadTexture("../data/vanquish/upper_d.tga", location);
		m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", location);
		m_renderer->LoadSpecularMap("../data/vanquish/upper_d.tga", location);

		m_renderer->SetTransform(randomPos, location);

		//Visor
		location = m_renderer->LoadOBJ("../data/vanquish/visor.obj");
		m_renderer->LoadAmbient("../data/vanquish/visor.tga", location);
		m_renderer->LoadTexture("../data/vanquish/visor.tga", location);
		m_renderer->LoadSpecularMap("../data/vanquish/visor.tga", location);

		m_renderer->SetTransform(randomPos, location);

		//Ground
		//location = m_renderer->GenerateGrid(10, 10);
		//m_renderer->LoadTexture("../data/crate.png", location);
		//m_renderer->LoadNormalMap("../data/rock_normal.tga", location);
		//
		//m_renderer->SetTransform(randomPos, location);
	}
	

	//m_renderer->LoadOBJ("../data/teapot.obj");

	//std::vector<std::string> textures;
	//std::vector<std::string> normalMaps;
	//std::vector<std::string> specularMaps;
	//std::vector<bool> texChannels;
	//std::vector<bool> normChannels;
	//std::vector<bool> specularChannels;
	//
	//textures.push_back("../data/Enemyelite/EnemyElite3_D.tga");
	//textures.push_back("../data/Enemyelite/Alienrifle_D.png");
	//normalMaps.push_back("../data/Enemyelite/EnemyElite_N.tga");
	//normalMaps.push_back("../data/Enemyelite/Alienrifle_N.png");
	//specularMaps.push_back("../data/Enemyelite/EnemyElite_S.tga");
	//specularMaps.push_back("../data/Enemyelite/Alienrifle_S.tga");
    //
	//m_model = m_renderer->LoadFBX("../data/Enemyelite/EnemyElite.fbx", &textures, &normalMaps, &specularMaps);

	m_renderer->GenerateShadowMap(50.0f);

	TwAddButton(m_debugBar, "Add Light", AddLight, (void*)m_renderer, "");

	m_timer = 0;

	return 0;
}

void Tutorial13::Update(float a_deltaTime)
{
	m_timer += a_deltaTime;
	m_camera->Update(a_deltaTime);
	m_renderer->UpdateAnimation(m_timer, m_model);
}

void Tutorial13::Draw()
{
	m_renderer->Draw();
}

int Tutorial13::Deinit()
{

	delete m_camera;

	m_renderer->CleanupBuffers();

	return Application::Deinit();
}