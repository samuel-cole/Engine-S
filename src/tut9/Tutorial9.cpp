#include "Tutorial9.h"
#include "glm\glm.hpp"
#include "glm\ext.hpp"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "glm\glm.hpp"

int Tutorial9::Init()
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

	unsigned int texture = m_renderer->LoadFrameBuffer(m_camera, vec4(0.0f, 0.0f, 512.0f, 512.0f), vec3(1.0f, 1.0f, 1.0f));

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

	unsigned int grid = m_renderer->GenerateGrid(1000, 1000, glm::vec3(0, 0, 0));
	m_renderer->LoadTexture(texture, grid);
	m_renderer->LoadNormalMap("../data/rock_normal.tga", grid);

	m_emitter = m_renderer->CreateEmitter(100000, 0.0f, 500.0f, 5.0f, 5.0f, 1.0f, 1.0f, vec4(1, 0, 0, 1), vec4(1, 1, 0, 1), true);

	m_timer = 0;

	return 0;
}

void Tutorial9::Update(float a_deltaTime)
{
	m_timer += a_deltaTime;
	m_camera->Update(a_deltaTime);
	m_renderer->UpdateAnimation(m_timer);
	m_renderer->UpdateEmitters(a_deltaTime);
}

void Tutorial9::Draw()
{
	m_renderer->Draw();
}

int Tutorial9::Deinit()
{
	delete m_camera;

	m_renderer->CleanupBuffers();

	m_renderer->DestroyEmitter(m_emitter, true);

	return Application::Deinit();
}