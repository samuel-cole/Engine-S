#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "glm\glm.hpp"
#include "AntTweakBar.h"
#include "GLFW\glfw3.h"

struct GLFWwindow;

enum Keys { UP = 265,
			DOWN = 264,
			LEFT = 263, 
			RIGHT = 262,
			SPACE = 32,
			W = 87,
			A = 65,
			S = 83,
			D = 68};

class InputManager
{
public:
	static bool GetKey(const Keys a_key);

	static bool GetMouseDown(const int a_mouseButton);

	static void SetMousePos(const glm::vec2& a_pos);

	static void SetMouseToCenter();

	static glm::vec2 GetMousePos();

	inline static void SetWindow(GLFWwindow* a_window)
	{
		m_window = a_window;
	}

	//Callbacks for AntTweakBar
	inline static void OnMouseButton(GLFWwindow*, int b, int a, int m)
	{
		TwEventMouseButtonGLFW(b, a);
	}
	inline static void OnMousePosition(GLFWwindow*, double x, double y)
	{
		TwEventMousePosGLFW((int)x, (int)y);
	}
	inline static void OnMouseScroll(GLFWwindow*, double x, double y)
	{
		TwEventMouseWheelGLFW((int)y);
	}
	inline static void OnKey(GLFWwindow*, int k, int s, int a, int m)
	{
		TwEventKeyGLFW(k, a);
	}
	inline static void OnChar(GLFWwindow*, unsigned int c)
	{
		TwEventCharGLFW(c, GLFW_PRESS);
	}
	inline static void OnWindowResize(GLFWwindow*, int w, int h)
	{
		TwWindowSize(w, h);
		glViewport(0, 0, w, h);
	}
	static void SetupAntBarCallbacks();

private:
	static GLFWwindow* m_window;
	
};

#endif