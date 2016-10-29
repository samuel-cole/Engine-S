#include "Application.h"
#include "gl_core_4_4.h"
#include "GLFW\glfw3.h"
#include "InputManager.h"
#include <time.h>

int Application::Run()
{
	int init = Init();
	if (init != 0)
		return init;

	while (glfwWindowShouldClose(m_window) == false && glfwGetKey(m_window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
	{
		m_currentTime = (float)glfwGetTime();
		m_deltaTime = m_currentTime - m_previousTime;
		m_previousTime = m_currentTime;

		Update(m_deltaTime);
		Draw();

		TwDraw();
		
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	return Deinit();
}

int Application::Init()
{
	if (glfwInit() == false)
		return -1;

	m_window = glfwCreateWindow(1280, 720, "FleX Physics Game", nullptr, nullptr);

	if (m_window == nullptr)
	{
		glfwTerminate();
		return -2;
	}

	glfwMakeContextCurrent(m_window);

	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
		return -3;
	}

	m_previousTime = 0.01f;

	TwInit(TW_OPENGL_CORE, nullptr);
	TwWindowSize(1280, 720);

	InputManager::SetWindow(m_window);
	InputManager::SetupAntBarCallbacks();

	m_debugBar = TwNewBar("Options Bar");
	float refreshRate = 0.01f;
	TwSetParam(m_debugBar, NULL, "refresh", TwParamValueType::TW_PARAM_FLOAT, 1, &refreshRate);

	srand((unsigned int)time(0));

	return 0;
}

int Application::Deinit()
{
	TwDeleteAllBars();
	TwTerminate();
	glfwDestroyWindow(m_window);
	glfwTerminate();
	return 0;
}