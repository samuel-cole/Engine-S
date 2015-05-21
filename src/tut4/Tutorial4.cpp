#include "Tutorial4.h"
#include "glm\glm.hpp"
#include "glm\ext.hpp"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "glm\glm.hpp"

#include <thread>
#include <vector>

int Tutorial4::Init()
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

	//Lower suit
	unsigned int location = m_renderer->LoadOBJ("../data/vanquish/lower.obj");
	m_renderer->LoadTexture("../data/vanquish/lower_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/lower_n.tga", location);

	//Upper suit
	location = m_renderer->LoadOBJ("../data/vanquish/upper.obj");
	m_renderer->LoadTexture("../data/vanquish/upper_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", location);

	//Inner suit
	location = m_renderer->LoadOBJ("../data/vanquish/inner.obj");
	m_renderer->LoadTexture("../data/vanquish/inner_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", location);

	//Hand
	location = m_renderer->LoadOBJ("../data/vanquish/hand.obj");
	m_renderer->LoadTexture("../data/vanquish/inner_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/inner_n.tga", location);

	//Buttons
	location = m_renderer->LoadOBJ("../data/vanquish/buttons.obj");
	m_renderer->LoadTexture("../data/vanquish/upper_d.tga", location);
	m_renderer->LoadNormalMap("../data/vanquish/upper_n.tga", location);

	//Visor
	location = m_renderer->LoadOBJ("../data/vanquish/visor.obj");
	m_renderer->LoadTexture("../data/vanquish/visor.tga", location);
	
	//Ground
	location = m_renderer->GenerateGrid(10, 10);
	m_renderer->LoadTexture("../data/crate.png", location);
	m_renderer->LoadNormalMap("../data/rock_normal.tga", location);

	m_renderer->CreatePointLight(vec3(1, 0, 0), 10, vec3(0, 1, 1));



	return 0;
}

void Tutorial4::Update(float a_deltaTime)
{
	m_camera->Update(a_deltaTime);
}

void Tutorial4::Draw()
{
	m_renderer->Draw();
}

int Tutorial4::Deinit()
{

	delete m_camera;

	m_renderer->CleanupBuffers();

	return Application::Deinit();
}