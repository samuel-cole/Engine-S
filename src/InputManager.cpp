#include "InputManager.h"
#include "GLFW\glfw3.h"

bool InputManager::GetKey(Keys a_key)
{
	return glfwGetKey(m_window, a_key) > 0;
}

bool InputManager::GetMouseDown(int a_mouseButton)
{
	return glfwGetMouseButton(m_window, a_mouseButton) > 0;
}

void InputManager::SetMousePos(glm::vec2 a_pos)
{
	glfwSetCursorPos(m_window, (double)a_pos.x, (double)a_pos.y);
}

void InputManager::SetMouseToCenter()
{
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);
	glfwSetCursorPos(m_window, (double)(width/2), (double)(height/2));
}

glm::vec2 InputManager::GetMousePos()
{
	double xPos, yPos;
	glfwGetCursorPos(m_window, &xPos, &yPos);
	return glm::vec2(xPos, yPos);
}

void InputManager::SetupAntBarCallbacks()
{
	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);
	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);
	glfwSetWindowSizeCallback(m_window, OnWindowResize);
}

GLFWwindow* InputManager::m_window = nullptr;