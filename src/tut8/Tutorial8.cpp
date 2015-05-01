#include "Tutorial8.h"
#include "glm\glm.hpp"
#include "glm\ext.hpp"
#include "FlyCamera.h"
#include "Renderer.h"

int Tutorial8::Init()
{
	int baseInit = Application::Init();
	if (baseInit != 0)
		return baseInit;

	m_camera = new FlyCamera(m_debugBar);
	m_camera->SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 10000.0f);
	m_camera->SetLookAt(vec3(10, 10, 10), vec3(0), vec3(0, 1, 0));

	m_renderer = new Renderer(m_camera, m_debugBar);

	particleDirection = true;

	m_emitter = m_renderer->CreateEmitter(100000,			//Max particles
										  0.1f,				//Lifespan minimum 
										  1.0f,			//Lifespan maximum
										  100.0f,			//Velocity minimum
										  100.0f,			//Velocity maximum
										  1.0f,				//Start size
										  0.1f,				//End size
										  vec4(1, 0, 0, 1), //Start colour
										  vec4(1, 1, 0, 1), //End colour
										  vec3(1, 0, 0),	//Direction
										  3.14159265358979f/10.0f,//Direction variance
										  true,
										  m_debugBar);			//GPU based

	m_renderer->GenerateGrid(10, 10);

	//m_emitter = m_renderer->CreateEmitter(100000,			//Max particles
	//									  0.1f,				//Lifespan minimum 
	//									  100.0f,			//Lifespan maximum
	//									  5.0f,				//Velocity minimum
	//									  5.0f,				//Velocity maximum
	//									  1.0f,				//Start size
	//									  0.1f,				//End size
	//									  vec4(1, 0, 0, 1), //Start colour
	//									  vec4(1, 1, 0, 1), //End colour
	//									  vec3(1, 0, 0),	//Direction
	//									  3.14159265358979f,//Direction variance
	//									  true);			//GPU based
   
	//m_emitter = m_renderer->CreateEmitter(1000000,			//Max particles
	//									  0.1f,				//Lifespan minimum 
	//									  10.0f,			//Lifespan maximum
	//									  0.0f,				//Velocity minimum
	//									  1.0f,				//Velocity maximum
	//									  0.1f,				//Start size
	//									  0.1f,				//End size
	//									  vec4(1, 0, 0, 1), //Start colour
	//									  vec4(1, 1, 0, 1), //End colour
	//									  vec3(1, 0, 0),	//Direction
	//									  3.14159265358979f,//Direction variance
	//									  true);			//GPU based
	//
	//m_emitter2 = m_renderer->CreateEmitter(1000000,			 //Max particles
	//									   0.1f,			 //Lifespan minimum 
	//									   10.0f,			 //Lifespan maximum
	//									   0.0f,			 //Velocity minimum
	//									   1.0f,			 //Velocity maximum
	//									   0.1f,			 //Start size
	//									   0.1f,			 //End size
	//									   vec4(0, 0, 1, 1), //Start colour
	//									   vec4(0, 1, 1, 1), //End colour
	//									   vec3(1, 0, 0),	 //Direction
	//									   3.14159265358979f,//Direction variance
	//									   true);			 //GPU based
	//
	//m_emitter3 = m_renderer->CreateEmitter(100000,			 //Max particles
	//									   0.1f,			 //Lifespan minimum 
	//									   10.0f,			 //Lifespan maximum
	//									   0.0f,			 //Velocity minimum
	//									   0.0f,			 //Velocity maximum
	//									   0.1f,			 //Start size
	//									   0.1f,			 //End size
	//									   vec4(1, 1, 1, 1), //Start colour
	//									   vec4(1, 1, 1, 1), //End colour
	//									   vec3(1, 0, 0),	//Direction
	//									   3.14159265358979f,//Direction variance
	//									   true);			 //GPU based

	m_timer = 0;

	return 0;
}

void Tutorial8::Update(float a_deltaTime)
{
	m_timer += a_deltaTime;

	m_camera->Update(a_deltaTime);

	//if (m_renderer->GetEmitterPosition(m_emitter, true).y > 40.0f)
	//	particleDirection = false;
	//else if (m_renderer->GetEmitterPosition(m_emitter, true).y < -40.0f)
	//	particleDirection = true;
	//
	//m_renderer->SetEmitterPosition(m_emitter,  true, vec3(cosf(m_timer),				 m_renderer->GetEmitterPosition(m_emitter,  true).y + ((particleDirection) ? 0.1f : -0.1f), sinf(m_timer)));
	//m_renderer->SetEmitterPosition(m_emitter2, true, vec3(cosf(m_timer + 3.1415926535f), m_renderer->GetEmitterPosition(m_emitter2, true).y + ((particleDirection) ? 0.1f : -0.1f), sinf(m_timer + 3.1415926535f)));
	//m_renderer->SetEmitterPosition(m_emitter3, true, vec3(0, m_renderer->GetEmitterPosition(m_emitter3, true).y + ((particleDirection) ? 0.1f : -0.1f), 0));
}

void Tutorial8::Draw()
{
	m_renderer->Draw();
}

int Tutorial8::Deinit()
{

	delete m_camera;

	m_renderer->CleanupBuffers();

	m_renderer->DestroyEmitter(m_emitter, true);


	return Application::Deinit();
}