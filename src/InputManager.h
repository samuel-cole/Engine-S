#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "glm\glm.hpp"
#include "AntTweakBar.h"
#include "GLFW\glfw3.h"

struct GLFWwindow;

enum Keys 
{ 
	UP = GLFW_KEY_UP,
	DOWN = GLFW_KEY_DOWN,
	LEFT = GLFW_KEY_LEFT,
	RIGHT = GLFW_KEY_RIGHT,
	SPACE = GLFW_KEY_SPACE,
	ENTER = GLFW_KEY_ENTER,
	W = GLFW_KEY_W,
	A = GLFW_KEY_A,
	S = GLFW_KEY_S,
	D = GLFW_KEY_D,
	R = GLFW_KEY_R,
	NUM_1 = GLFW_KEY_1,
	NUM_2 = GLFW_KEY_2,
	NUM_3 = GLFW_KEY_3,
	NUM_4 = GLFW_KEY_4,
	NUM_5 = GLFW_KEY_5
};

//Class used for managing input. Wraps some GLFW input handling.
class InputManager
{
public:
	//Gets whether the specified key is currently down.
	static bool GetKey(const Keys a_key);

	//=======================================================
	//FleX project addition
	//=======================================================
	//Gets whether the specified key was pressed down this frame.
	static bool GetKeyDown(const Keys a_key);
	//=======================================================
	//End FleX project addition
	//=======================================================

	//Get whether the mouse button passed in as an argument is being pressed. Starts at 0 (for left click).
	static bool GetMouseDown(const int a_mouseButton);

	static void SetMousePos(const glm::vec2& a_pos);

	static void SetMouseToCenter();

	static glm::vec2 GetMousePos();

	static void SetMouseVisibility(const bool a_visible);

	inline static void SetWindow(GLFWwindow* a_window)
	{
		s_window = a_window;
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

	//=======================================================
	//FleX project modification:
	//This function previously only handled sending information to Anttweakbar, however it now also updates keysPressedThisFrame. 
	//=======================================================
	//Callback function for glfw on key input events.
	//Sends information to Anttweakbar, and updates the keys pressed this frame.
	static void OnKey(GLFWwindow*, int k, int s, int a, int m);
	//=======================================================
	//End FleX project modification
	//=======================================================

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

	inline static void Update()
	{
		keysPressedThisFrame = 0;
	}

private:

	//=======================================================
	//FleX project addition
	//=======================================================
	//Enum used to specify which bits should be used for which keys within the keysPressedThisFrame bitmask.
	enum KeyMasks
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		SPACE,
		ENTER,
		W,
		A,
		S,
		D,
		R,
		NUM_1,
		NUM_2,
		NUM_3,
		NUM_4,
		NUM_5
	};
	
	//Bitmask for which keys have been pushed this frame.
	static int keysPressedThisFrame;
	//=======================================================
	//End FleX project addition
	//=======================================================

	static GLFWwindow* s_window;
};

#endif