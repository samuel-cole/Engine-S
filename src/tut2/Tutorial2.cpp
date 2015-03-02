#include "Tutorial2.h"
#include "Gizmos.h"
#include "glm\glm.hpp"
#include "glm\ext.hpp"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "FlyCamera.h"

using glm::mat4;
using glm::vec3;
using glm::vec4;

int Tutorial2::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	Gizmos::create();
	
	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f/9.0f, 0.1f, 1000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));


	
	auto major = ogl_GetMajorVersion();
	auto minor = ogl_GetMinorVersion();
	printf("GL: %i.%i\n", major, minor);

	return 0;
}

void Tutorial2::Update(float a_deltaTime)
{
	Gizmos::clear();
	Gizmos::addTransform(mat4(1));
	
	vec4 white(1);
	vec4 black(0, 0, 0, 1);
	

	m_camera->Update(a_deltaTime);
	
	for (int i = 0; i < 21; ++i)
	{
		Gizmos::addLine(vec3(-10 + i, 0, 10),
						vec3(-10 + i, 0, -10),
						i == 10 ? white : black);
	
		Gizmos::addLine(vec3(10, 0, -10 + i),
						vec3(-10, 0, -10 + i),
						i == 10 ? white : black);
	}
}

void Tutorial2::Draw()
{
	Gizmos::draw(m_camera->GetProjectionView());
}

int Tutorial2::Deinit()
{
	Gizmos::destroy();

	delete m_camera;

	return Application::Deinit();
}