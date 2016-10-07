#include "InputManager.h"
#include "GLFW\glfw3.h"

bool InputManager::GetKey(Keys a_key)
{
	return glfwGetKey(s_window, a_key) > 0;
}

bool InputManager::GetKeyDown(Keys a_key)
{
	switch (a_key)
	{
	case Keys::UP:
		return (keysPressedThisFrame & (1 << KeyMasks::UP)) > 0;
	case Keys::DOWN:
		return (keysPressedThisFrame & (1 << KeyMasks::DOWN)) > 0;
	case Keys::LEFT:
		return (keysPressedThisFrame & (1 << KeyMasks::LEFT)) > 0;
	case Keys::RIGHT:
		return (keysPressedThisFrame & (1 << KeyMasks::RIGHT)) > 0;
	case Keys::SPACE:
		return (keysPressedThisFrame & (1 << KeyMasks::SPACE)) > 0;
	case Keys::ENTER:
		return (keysPressedThisFrame & (1 << KeyMasks::ENTER)) > 0;
	case Keys::W:
		return (keysPressedThisFrame & (1 << KeyMasks::W)) > 0;
	case Keys::A:
		return (keysPressedThisFrame & (1 << KeyMasks::A)) > 0;
	case Keys::S:
		return (keysPressedThisFrame & (1 << KeyMasks::S)) > 0;
	case Keys::D:
		return (keysPressedThisFrame & (1 << KeyMasks::D)) > 0;
	}
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

void InputManager::OnKey(GLFWwindow*, int keycode, int scancode, int action, int mmods)
{
	TwEventKeyGLFW(keycode, action);

	if (action == GLFW_PRESS)
	{
		switch ((Keys)keycode)
		{
		case Keys::UP:
			keysPressedThisFrame |= (1 << KeyMasks::UP);
			break;
		case Keys::DOWN:
			keysPressedThisFrame |= (1 << KeyMasks::DOWN);
			break;
		case Keys::LEFT:
			keysPressedThisFrame |= (1 << KeyMasks::LEFT);
			break;
		case Keys::RIGHT:
			keysPressedThisFrame |= (1 << KeyMasks::RIGHT);
			break;
		case Keys::SPACE:
			keysPressedThisFrame |= (1 << KeyMasks::SPACE);
			break;
		case Keys::ENTER:
			keysPressedThisFrame |= (1 << KeyMasks::ENTER);
			break;
		case Keys::W:
			keysPressedThisFrame |= (1 << KeyMasks::W);
			break;
		case Keys::A:
			keysPressedThisFrame |= (1 << KeyMasks::A);
			break;
		case Keys::S:
			keysPressedThisFrame |= (1 << KeyMasks::S);
			break;
		case Keys::D:
			keysPressedThisFrame |= (1 << KeyMasks::D);
			break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		switch ((Keys)keycode)
		{
		case Keys::UP:
			keysPressedThisFrame &= ~(1 << KeyMasks::UP);
			break;
		case Keys::DOWN:
			keysPressedThisFrame &= ~(1 << KeyMasks::DOWN);
			break;
		case Keys::LEFT:
			keysPressedThisFrame &= ~(1 << KeyMasks::LEFT);
			break;
		case Keys::RIGHT:
			keysPressedThisFrame &= ~(1 << KeyMasks::RIGHT);
			break;
		case Keys::SPACE:
			keysPressedThisFrame &= ~(1 << KeyMasks::SPACE);
			break;
		case Keys::ENTER:
			keysPressedThisFrame &= ~(1 << KeyMasks::ENTER);
			break;
		case Keys::W:
			keysPressedThisFrame &= ~(1 << KeyMasks::W);
			break;
		case Keys::A:
			keysPressedThisFrame &= ~(1 << KeyMasks::A);
			break;
		case Keys::S:
			keysPressedThisFrame &= ~(1 << KeyMasks::S);
			break;
		case Keys::D:
			keysPressedThisFrame &= ~(1 << KeyMasks::D);
			break;
		}
	}

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
int InputManager::keysPressedThisFrame = 0;