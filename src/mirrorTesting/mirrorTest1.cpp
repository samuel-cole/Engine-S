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

	//Back wall
	m_mirror = m_renderer->MakeMirror(20, 20, vec4(0.0f, 0.0f, 512.0f, 512.0f), vec3(1.0f, 1.0f, 1.0f));
	m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_mirror), vec3(0, 5, -10)), m_mirror);
	m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), 3.14159265358979f / 2.0f, vec3(1.0f, 0, 0)), m_mirror);
	m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), 3.14159265358979f / 2.0f, vec3(0, 1.0f, 0)), m_mirror);

	//Front wall
	//m_mirror = m_renderer->MakeMirror(20, 20, vec4(0.0f, 0.0f, 512.0f, 512.0f), vec3(1.0f, 1.0f, 1.0f));
	//m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_mirror), vec3(0, 5, 10)), m_mirror);
	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), 3.14159265358979f / 2.0f, vec3(1.0f, 0, 0)), m_mirror);
	//
	////Right wall
	//m_mirror = m_renderer->MakeMirror(20, 20, vec4(0.0f, 0.0f, 512.0f, 512.0f), vec3(1.0f, 1.0f, 1.0f));
	//m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_mirror), vec3(10, 5, 0)), m_mirror);
	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), 3.14159265358979f / 2.0f, vec3(1.0f, 0, 0)), m_mirror);
	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), 3.14159265358979f / 2.0f, vec3(0, 0.0f, 1.0f)), m_mirror);
	//
	////Left wall
	//m_mirror = m_renderer->MakeMirror(20, 20, vec4(0.0f, 0.0f, 512.0f, 512.0f), vec3(1.0f, 1.0f, 1.0f));
	//m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_mirror), vec3(-10, 5, 0)), m_mirror);
	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), 3.14159265358979f / 2.0f, vec3(1.0f, 0, 0)), m_mirror);
	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), -3.14159265358979f / 2.0f, vec3(0, 0.0f, 1.0f)), m_mirror);
	//
	////Floor
	//m_mirror = m_renderer->MakeMirror(20, 20, vec4(0.0f, 0.0f, 512.0f, 512.0f), vec3(1.0f, 1.0f, 1.0f));
	//m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(m_mirror), vec3(0, 0, 0)), m_mirror);


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

	m_renderer->GenerateShadowMap(50.0f);

	m_timer = 0;

	return 0;
}

void MirrorTest1::Update(float a_deltaTime)
{
	m_timer += a_deltaTime;

	

	m_renderer->UpdateMirrors();

	//m_staticCamera->SetLookAt(reflectedPoint, mirrorPosition, vec3(0, 1, 0));
	
	m_camera->Update(a_deltaTime);
	//m_renderer->UpdateAnimation(m_timer, m_model);

	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_mirror), a_deltaTime, vec3(1.0f, 0, 0)), m_mirror);
	//m_renderer->SetTransform(glm::translate(m_renderer->GetTransform(3), vec3(a_deltaTime, 0, 0)), 3);
	//m_renderer->SetTransform(glm::rotate(m_renderer->GetTransform(m_model), a_deltaTime, vec3(0, 1, 0)), m_model);
}

void MirrorTest1::Draw()
{
	m_renderer->Draw();
}

int MirrorTest1::Deinit()
{
	delete m_camera;
	
	m_renderer->CleanupBuffers();

	return Application::Deinit();
}