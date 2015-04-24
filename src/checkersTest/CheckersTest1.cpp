#include "CheckersTest1.h"
#include "glm\ext.hpp"
#include "StaticCamera.h"
#include "FlyCamera.h"
#include "Renderer.h"
#include "AntTweakBar.h"

void TW_CALL BoardGenerate(void* a_clientData)
{
	InfoForBar* infoForBar = (InfoForBar*)a_clientData;

	infoForBar->renderer->DestroyObject(infoForBar->object);
	infoForBar->object = infoForBar->renderer->GenerateGrid(infoForBar->rows, infoForBar->columns);
	infoForBar->renderer->LoadTexture("../data/checkerboard.png", infoForBar->object);
	infoForBar->renderer->GeneratePerlinNoiseMap(infoForBar->perlinRows, infoForBar->perlinColumns, 6, infoForBar->amplitude, infoForBar->persistence, infoForBar->object, infoForBar->seed, false);
}

int CheckersTest::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0, 0, 0), vec3(0, 1, 0));

	m_infoForBar.renderer = new Renderer(m_camera, m_debugBar);

	m_infoForBar.amplitude = 3.0f;
	m_infoForBar.persistence = 0.3f;
	m_infoForBar.seed = 0;
	m_infoForBar.rows = 100;
	m_infoForBar.columns = 100;
	m_infoForBar.perlinRows = 9;
	m_infoForBar.perlinColumns = 9;
	m_infoForBar.object = m_infoForBar.renderer->GenerateGrid(m_infoForBar.rows, m_infoForBar.columns);
	m_infoForBar.renderer->LoadTexture("../data/checkerboard.png", m_infoForBar.object);
	m_infoForBar.renderer->GeneratePerlinNoiseMap(m_infoForBar.perlinRows, m_infoForBar.perlinColumns, 6, m_infoForBar.amplitude, m_infoForBar.persistence, m_infoForBar.object, m_infoForBar.seed, false);

	m_tileWidth = m_infoForBar.rows / 8.0f;
	
	TwAddSeparator(m_debugBar, "Plane 1", "");
	TwAddVarRW(m_debugBar, "Amplitude", TW_TYPE_FLOAT, &m_infoForBar.amplitude, "");
	TwAddVarRW(m_debugBar, "Persistence", TW_TYPE_FLOAT, &m_infoForBar.persistence, "");
	TwAddVarRW(m_debugBar, "Seed", TW_TYPE_UINT32, &m_infoForBar.seed, "");
	TwAddVarRW(m_debugBar, "Rows", TW_TYPE_UINT32, &m_infoForBar.rows, "");
	TwAddVarRW(m_debugBar, "Columns", TW_TYPE_UINT32, &m_infoForBar.columns, "");
	TwAddVarRW(m_debugBar, "Perlin Rows", TW_TYPE_UINT32, &m_infoForBar.perlinRows, "");
	TwAddVarRW(m_debugBar, "Perlin Columns", TW_TYPE_UINT32, &m_infoForBar.perlinColumns, "");
	TwAddButton(m_debugBar, "Re-generate", BoardGenerate, (void*)&m_infoForBar, "");

	for (unsigned int i = 0; i < 24; ++i)
	{
		unsigned int emitter = m_infoForBar.renderer->CreateEmitter(1000, //Max particles
			0.1f,			//Lifespan minimum 
			10.0f,			//Lifespan maximum
			0.1f,			//Velocity minimum
			1.0f,			//Velocity maximum
			1.0f,			//Start size
			0.1f,			//End size
			((i < 12) ? vec4(1, 0, 0, 1) : vec4(0, 0, 1, 1)), //Start colour
			((i < 12) ? vec4(1, 1, 0, 1) : vec4(0, 1, 1, 1)), //End colour
			vec3(1, 0, 0),	//Direction
			3.14159265358979f,//Direction variance
			true,
			m_debugBar);			//GPU based

		m_emitters.push_back(emitter);
	}

	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i], true, vec3(m_tileWidth * -3.5f + i * 2 * m_tileWidth, 5, m_tileWidth * -2.5f));
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 12], true, vec3(m_tileWidth * -3.5f + i * 2 * m_tileWidth, 5, m_tileWidth * 1.5f));
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 16], true, vec3(m_tileWidth * -3.5f + i * 2 * m_tileWidth, 5, m_tileWidth * 3.5f));
	}

	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 4], true, vec3(m_tileWidth * -2.5f + i * 2 * m_tileWidth, 5, m_tileWidth * -3.5f));
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 8], true, vec3(m_tileWidth * -2.5f + i * 2 * m_tileWidth, 5, m_tileWidth * -1.5f));
	}
	for (unsigned int i = 0; i < 4; ++i)
	{
		m_infoForBar.renderer->SetEmitterPosition(m_emitters[i + 20], true, vec3(m_tileWidth * -2.5f + i * 2 * m_tileWidth, 5, m_tileWidth * 2.5f));
	}


	return 0;
}

void CheckersTest::Update(float a_deltaTime)
{
	m_camera->Update(a_deltaTime);

	m_tileWidth = m_infoForBar.rows / 8.0f;
}

void CheckersTest::Draw()
{
	m_infoForBar.renderer->Draw();
}

int CheckersTest::Deinit()
{
	delete m_camera;

	m_infoForBar.renderer->CleanupBuffers();

	return Application::Deinit();
}