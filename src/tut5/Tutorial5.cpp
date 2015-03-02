#include "Tutorial5.h"
#include "glm\glm.hpp"
#include "glm\ext.hpp"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "FlyCamera.h"
#include "Renderer.h"

int Tutorial5::Init()
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
	
	m_renderer->GenerateGrid(10, 10);

	m_renderer->LoadTexture("../data/rock_diffuse.tga", false);

	m_renderer->LoadNormalMap("../data/rock_normal.tga", false);



	return 0;
}

void Tutorial5::Update(float a_deltaTime)
{
	m_camera->Update(a_deltaTime);
}

void Tutorial5::Draw()
{
	m_renderer->Draw();
}

int Tutorial5::Deinit()
{

	delete m_camera;

	m_renderer->CleanupBuffers();

	return Application::Deinit();
}