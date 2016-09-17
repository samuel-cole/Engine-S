#include <iostream>
#include "glm\ext.hpp"
#include "WalkCamera.h"
#include "InputManager.h"
#include "AntTweakBar.h"

using glm::vec4;

WalkCamera::WalkCamera(TwBar* a_debugBar) : m_up(0, 1, 0), m_checkFrame(-1), m_mouseStartPos(0, 0)
{
}

void WalkCamera::Update(float a_deltaTime)
{
	if (InputManager::GetMouseDown(1))
	{
		InputManager::SetMouseVisibility(false);
		if (m_checkFrame == 0)
		{
			InputManager::SetMouseToCenter();
			m_mouseStartPos = InputManager::GetMousePos();
		}
		else if (m_checkFrame == 2)
		{
			glm::vec2 offset = InputManager::GetMousePos() - m_mouseStartPos;
			SetRotationFromMouseOffset(offset * 10.0f, a_deltaTime);
		}
		++m_checkFrame;
		if (m_checkFrame == 3)
			m_checkFrame = 0;
	}
	else
	{
		InputManager::SetMouseVisibility(true);
	}
}

void WalkCamera::SetRotationFromMouseOffset(const glm::vec2& a_offset, float a_deltaTime)
{
	vec3 rightVec = glm::vec3(1, 0, 0);
	vec3 upVec = glm::vec3(0, 1, 0);

	glm::mat4 xRot = glm::rotate(-a_offset.x * a_deltaTime * 0.01f, upVec);
	glm::mat4 yRot = glm::rotate(-a_offset.y * a_deltaTime * 0.01f, rightVec);

	if (glm::length(a_offset) > 0.01f)
		SetWorldTransform(GetWorldTransform() * (xRot * yRot));

	//warping occuring here because they are no longer normalized or perpendicular (orthonormal).
	//z axis is the one we want to keep the same
	//the right vector should then be calculated by crossing the worlds up and the z.
	//the camera up vector should then be calculated by crossing the right and z.
	//remember that the order matters with these- I may have it wrong.
	rightVec = glm::cross(m_up, (vec3)(GetWorldTransform()[2]));
	upVec = glm::cross((vec3)(GetWorldTransform()[2]), rightVec);

	SetZRotation((vec3)glm::normalize(GetWorldTransform()[2]));
	rightVec = glm::normalize(rightVec);
	upVec = glm::normalize(upVec);

	SetXRotation(rightVec);
	SetYRotation(upVec);
}