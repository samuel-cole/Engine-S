#include "InputManager.h"
#include "GLFW\glfw3.h"

bool InputManager::GetKey(Keys a_key)
{
	return glfwGetKey(s_window, a_key) > 0;
}

bool InputManager::GetMouseDown(int a_mouseButton)
{
	return glfwGetMouseButton(s_window, a_mouseButton) > 0;
}

void InputManager::SetMousePos(const glm::vec2& a_pos)
{
	glfwSetCursorPos(s_window, (double)a_pos.x, (double)a_pos.y);
}

void InputManager::SetMouseVisibility(const bool a_visible)
{
	if ((glfwGetInputMode(s_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) != a_visible)
		glfwSetInputMode(s_window, GLFW_CURSOR, (a_visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED));
}

glm::vec2 InputManager::GetMousePos()
{
	double xPos, yPos;
	glfwGetCursorPos(s_window, &xPos, &yPos);
	return glm::vec2(xPos, yPos);
}

void InputManager::SetMouseToCenter()
{
	int width, height;
	glfwGetFramebufferSize(s_window, &width, &height);
	glfwSetCursorPos(s_window, (double)(width/2), (double)(height/2));
}

void InputManager::SetupAntBarCallbacks()
{
	glfwSetMouseButtonCallback(s_window, OnMouseButton);
	glfwSetCursorPosCallback(s_window, OnMousePosition);
	glfwSetScrollCallback(s_window, OnMouseScroll);
	glfwSetKeyCallback(s_window, OnKey);
	glfwSetCharCallback(s_window, OnChar);
	glfwSetWindowSizeCallback(s_window, OnWindowResize);
}

GLFWwindow* InputManager::s_window = nullptr;