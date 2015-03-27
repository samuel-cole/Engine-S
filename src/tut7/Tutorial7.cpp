#include "Tutorial7.h"
#include "glm\glm.hpp"
#include "glm\ext.hpp"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "glm\glm.hpp"

int Tutorial7::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));



	auto major = ogl_GetMajorVersion();
	auto minor = ogl_GetMinorVersion();
	printf("GL: %i.%i\n", major, minor);

	m_renderer = new Renderer(m_camera, m_debugBar);


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

	m_renderer->LoadFBX("../data/Enemyelite/EnemyElite.fbx", &textures, &normalMaps, &specularMaps);

	m_emitter = m_renderer->CreateEmitter(1000, 500, 0.1f, 1.0f, 1, 5, 1, 0.1f, vec4(1, 0, 0, 1), vec4(1, 1, 0, 1), false);
	m_emitter2 = m_renderer->CreateEmitter(1000, 500, 0.1f, 2.0f, 1, 5, 1, 0.1f, vec4(0, 0, 1, 1), vec4(1, 1, 1, 1), false);
	

	m_timer = 0;

	return 0;
}

void Tutorial7::Update(float a_deltaTime)
{
	m_timer += a_deltaTime;

	m_camera->Update(a_deltaTime);

	m_renderer->UpdateEmitters(a_deltaTime);

	m_renderer->UpdateAnimation(m_timer);

}

void Tutorial7::Draw()
{
	m_renderer->Draw();
}

int Tutorial7::Deinit()
{

	delete m_camera;

	m_renderer->CleanupBuffers();

	m_renderer->DestroyEmitter(m_emitter, false);
	m_renderer->DestroyEmitter(m_emitter2, false);

	return Application::Deinit();
}