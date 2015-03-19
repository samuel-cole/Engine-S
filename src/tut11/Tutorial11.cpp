#include "Tutorial11.h"
#include "glm\ext.hpp"
#include "StaticCamera.h"
#include "FlyCamera.h"
#include "Renderer.h"


int Tutorial11::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);

	//Lower suit
	unsigned int location = m_renderer->LoadOBJ("../data/vanquish/lower.obj");
	m_renderer->LoadTexture("../data/vanquish/lower_d.tga", true, location);
	m_renderer->LoadNormalMap("../data/vanquish/lower_n.tga", true, location);

	//Upper suit
	location = m_renderer->LoadOBJ("../data/vanquish/upper.obj");
	m_renderer->LoadTexture("../data/vanquish/upper_d.tga", true, location);
	m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", true, location);

	//Inner suit
	location = m_renderer->LoadOBJ("../data/vanquish/inner.obj");
	m_renderer->LoadTexture("../data/vanquish/inner_d.tga", true, location);
	m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", true, location);

	//Hand
	location = m_renderer->LoadOBJ("../data/vanquish/hand.obj");
	m_renderer->LoadTexture("../data/vanquish/inner_d.tga", true, location);
	m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", true, location);

	//Buttons
	location = m_renderer->LoadOBJ("../data/vanquish/buttons.obj");
	m_renderer->LoadTexture("../data/vanquish/upper_d.tga", true, location);
	m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", true, location);

	//Visor
	location = m_renderer->LoadOBJ("../data/vanquish/visor.obj");
	m_renderer->LoadTexture("../data/vanquish/visor.tga", true, location);

	//Ground
	location = m_renderer->GenerateGrid(100, 100, glm::vec3(-50, 0, -50));
	m_renderer->LoadTexture("../data/crate.png", false, location);
	//m_renderer->LoadNormalMap("../data/rock_normal.tga", false, location);

	return 0;
}

void Tutorial11::Update(float a_deltaTime)
{
	m_camera->Update(a_deltaTime);
}

void Tutorial11::Draw()
{
	m_renderer->Draw();
}

int Tutorial11::Deinit()
{
	delete m_camera;

	m_renderer->CleanupBuffers();

	return Application::Deinit();
}