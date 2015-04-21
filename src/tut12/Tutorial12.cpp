#include "Tutorial12.h"
#include "glm\ext.hpp"
#include "StaticCamera.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "AntTweakBar.h"

void TW_CALL Regenerate(void* a_clientData)
{
	ButtonInfo* buttonInfo = (ButtonInfo*)a_clientData;

	buttonInfo->renderer->DestroyObject(buttonInfo->object);
	buttonInfo->object = buttonInfo->renderer->GenerateGrid(buttonInfo->rows, buttonInfo->columns);
	buttonInfo->renderer->LoadTexture("../data/crate.png", buttonInfo->object);
	buttonInfo->renderer->GeneratePerlinNoiseMap(buttonInfo->perlinRows, buttonInfo->perlinColumns, 6, buttonInfo->amplitude, buttonInfo->persistence, buttonInfo->object, buttonInfo->seed);
	buttonInfo->renderer->SetTransform(glm::translate(vec3(0, buttonInfo->height, 0)), buttonInfo->object);

	//buttonInfo->renderer->DestroyObject(buttonInfo->object);
	//buttonInfo->object = buttonInfo->renderer->LoadOBJ("../data/sphere/sphere.obj");
	//buttonInfo->renderer->LoadTexture("../data/vanquish/upper_d.tga", buttonInfo->object);
	//buttonInfo->renderer->LoadNormalMap("../data/vanquish/upper_n.tga", buttonInfo->object);
	//buttonInfo->renderer->GeneratePerlinNoiseMap(buttonInfo->perlinRows, buttonInfo->perlinColumns, 6, buttonInfo->amplitude, buttonInfo->persistence, buttonInfo->object, buttonInfo->seed);
}

void TW_CALL Regenerate2(void* a_clientData)
{
	ButtonInfo* buttonInfo = (ButtonInfo*)a_clientData;

	buttonInfo->renderer->DestroyObject(buttonInfo->object);
	buttonInfo->object = buttonInfo->renderer->GenerateGrid(buttonInfo->rows, buttonInfo->columns);
	buttonInfo->renderer->LoadTexture("../data/grey.png", buttonInfo->object);
	buttonInfo->renderer->GeneratePerlinNoiseMap(buttonInfo->perlinRows, buttonInfo->perlinColumns, 6, buttonInfo->amplitude, buttonInfo->persistence, buttonInfo->object, buttonInfo->seed);
	buttonInfo->renderer->SetTransform(glm::translate(vec3(0, buttonInfo->height, 0)), buttonInfo->object);
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
	m_buttonInfo2.renderer = m_buttonInfo.renderer;


	//Plane1
	m_buttonInfo.amplitude = 1.0f;
	m_buttonInfo.persistence = 0.3f;
	m_buttonInfo.seed = 0;
	m_buttonInfo.rows = 100;
	m_buttonInfo.columns = 100;
	m_buttonInfo.perlinRows = 100;
	m_buttonInfo.perlinColumns = 100;
	m_buttonInfo.height = 0;
	m_buttonInfo.object = m_buttonInfo.renderer->GenerateGrid(m_buttonInfo.rows, m_buttonInfo.columns);
	m_buttonInfo.renderer->LoadTexture("../data/crate.png", m_buttonInfo.object);
	m_buttonInfo.renderer->GeneratePerlinNoiseMap(m_buttonInfo.perlinRows, m_buttonInfo.perlinColumns, 6, m_buttonInfo.amplitude, m_buttonInfo.persistence, m_buttonInfo.object, m_buttonInfo.seed);
	
	TwAddSeparator(m_debugBar, "Plane 1", "");
	TwAddVarRW(m_debugBar, "Amplitude", TW_TYPE_FLOAT, &m_buttonInfo.amplitude, "");
	TwAddVarRW(m_debugBar, "Persistence", TW_TYPE_FLOAT, &m_buttonInfo.persistence, "");
	TwAddVarRW(m_debugBar, "Seed", TW_TYPE_UINT32, &m_buttonInfo.seed, "");
	TwAddVarRW(m_debugBar, "Rows", TW_TYPE_UINT32, &m_buttonInfo.rows, "");
	TwAddVarRW(m_debugBar, "Columns", TW_TYPE_UINT32, &m_buttonInfo.columns, "");
	TwAddVarRW(m_debugBar, "Perlin Rows", TW_TYPE_UINT32, &m_buttonInfo.perlinRows, "");
	TwAddVarRW(m_debugBar, "Perlin Columns", TW_TYPE_UINT32, &m_buttonInfo.perlinColumns, "");
	TwAddVarRW(m_debugBar, "Height", TW_TYPE_FLOAT, &m_buttonInfo.height, "");
	TwAddButton(m_debugBar, "Re-generate", Regenerate, (void*)&m_buttonInfo, "");

	//Plane2
	m_buttonInfo2.amplitude = 20.0f;
	m_buttonInfo2.persistence = 1.5f;
	m_buttonInfo2.seed = 0;
	m_buttonInfo2.rows = 100;
	m_buttonInfo2.columns = 100;
	m_buttonInfo2.perlinRows = 10;
	m_buttonInfo2.perlinColumns = 10;
	m_buttonInfo2.height = 0;
	m_buttonInfo2.object = m_buttonInfo.renderer->GenerateGrid(m_buttonInfo2.rows, m_buttonInfo2.columns);
	m_buttonInfo2.renderer->LoadTexture("../data/grey.png", m_buttonInfo2.object);
	m_buttonInfo2.renderer->GeneratePerlinNoiseMap(m_buttonInfo2.perlinRows, m_buttonInfo2.perlinColumns, 6, m_buttonInfo2.amplitude, m_buttonInfo2.persistence, m_buttonInfo2.object, m_buttonInfo2.seed);
	
	TwAddSeparator(m_debugBar, "Plane 2", "");
	TwAddVarRW(m_debugBar, "Amplitude 2", TW_TYPE_FLOAT, &m_buttonInfo2.amplitude, "");
	TwAddVarRW(m_debugBar, "Persistence 2", TW_TYPE_FLOAT, &m_buttonInfo2.persistence, "");
	TwAddVarRW(m_debugBar, "Seed 2", TW_TYPE_UINT32, &m_buttonInfo2.seed, "");
	TwAddVarRW(m_debugBar, "Rows 2", TW_TYPE_UINT32, &m_buttonInfo2.rows, "");
	TwAddVarRW(m_debugBar, "Columns 2", TW_TYPE_UINT32, &m_buttonInfo2.columns, "");
	TwAddVarRW(m_debugBar, "Perlin Rows 2", TW_TYPE_UINT32, &m_buttonInfo2.perlinRows, "");
	TwAddVarRW(m_debugBar, "Perlin Columns 2", TW_TYPE_UINT32, &m_buttonInfo2.perlinColumns, "");
	TwAddVarRW(m_debugBar, "Height 2", TW_TYPE_FLOAT, &m_buttonInfo2.height, "");
	TwAddButton(m_debugBar, "Re-generate 2", Regenerate2, (void*)&m_buttonInfo2, "");


	//Planet
	//m_buttonInfo.amplitude = 0.01f;
	//m_buttonInfo.persistence = 1.5f;
	//m_buttonInfo.seed = 0;
	//m_buttonInfo.perlinRows = 100;
	//m_buttonInfo.perlinColumns = 100;
	//m_buttonInfo.object = m_buttonInfo.renderer->LoadOBJ("../data/sphere/sphere.obj");
	//m_buttonInfo.renderer->LoadTexture("../data/vanquish/upper_d.tga", m_buttonInfo.object);
	//m_buttonInfo.renderer->LoadNormalMap("../data/vanquish/upper_n.tga", m_buttonInfo.object);
	//m_buttonInfo.renderer->GeneratePerlinNoiseMap(m_buttonInfo.perlinRows, m_buttonInfo.perlinColumns, 6, m_buttonInfo.amplitude, m_buttonInfo.persistence, m_buttonInfo.object, m_buttonInfo.seed);
	//
	//TwAddSeparator(m_debugBar, "Procedural Generation", "");
	//TwAddVarRW(m_debugBar, "Amplitude", TW_TYPE_FLOAT, &m_buttonInfo.amplitude, "");
	//TwAddVarRW(m_debugBar, "Persistence", TW_TYPE_FLOAT, &m_buttonInfo.persistence, "");
	//TwAddVarRW(m_debugBar, "Seed", TW_TYPE_UINT32, &m_buttonInfo.seed, "");
	//TwAddVarRW(m_debugBar, "Perlin Rows", TW_TYPE_UINT32, &m_buttonInfo.perlinRows, "");
	//TwAddVarRW(m_debugBar, "Perlin Columns", TW_TYPE_UINT32, &m_buttonInfo.perlinColumns, "");
	//TwAddButton(m_debugBar, "Re-generate", Regenerate, (void*)&m_buttonInfo, "");
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