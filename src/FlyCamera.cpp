#include <iostream>
#include "glm\ext.hpp"
#include "FlyCamera.h"
#include "InputManager.h"
#include "AntTweakBar.h"

using glm::vec4;

FlyCamera::FlyCamera(TwBar* a_debugBar) : m_speed(10.0f), m_up(0, 1, 0), m_viewButtonClicked(false), m_mouseStartPos(0, 0)
{
	TwAddVarRW(a_debugBar, "Camera Speed", TW_TYPE_FLOAT, &m_speed, "");
}

void FlyCamera::Update(float a_deltaTime)
{

	vec3 rightVec = (vec3)GetWorldTransform()[0];
	vec3 upVec = (vec3)GetWorldTransform()[1];
	vec3 forwardVec = (vec3)GetWorldTransform()[2];

	vec3 moveDir(0.0f);
	if (InputManager::GetKey(Keys::UP) || InputManager::GetKey(Keys::W))
		moveDir -= forwardVec;
	if (InputManager::GetKey(Keys::DOWN) || InputManager::GetKey(Keys::S))
		moveDir += forwardVec;
	if (InputManager::GetKey(Keys::LEFT) || InputManager::GetKey(Keys::A))
		moveDir -= rightVec;
	if (InputManager::GetKey(Keys::RIGHT) || InputManager::GetKey(Keys::D))
		moveDir += rightVec;
	
	if (InputManager::GetKey(Keys::SPACE))
		moveDir += m_up;

	float value = glm::length(moveDir);
	
	if (glm::length(moveDir) > 0.01f)
		moveDir = glm::normalize(moveDir) * (float)a_deltaTime * m_speed;

	SetPosition((vec3)(GetWorldTransform()[3]) + moveDir);

	
	if (InputManager::GetMouseDown(1))
	{
		if (!m_viewButtonClicked)
		{
			//InputManager::SetMouseToCenter();
			m_mouseStartPos = InputManager::GetMousePos();
			m_viewButtonClicked = true;
		}
		else
		{
			SetRotationFromMouseOffset(InputManager::GetMousePos() - m_mouseStartPos, a_deltaTime);
		}
	}
	else
	{
		m_viewButtonClicked = false;
	}

	
}

void FlyCamera::SetRotationFromMouseOffset(const glm::vec2& a_offset, float a_deltaTime)
{
	vec3 rightVec = glm::vec3(1, 0, 0); 
	vec3 upVec = glm::vec3(0, 1, 0);

	glm::mat4 xRot = glm::rotate( -a_offset.x * a_deltaTime * 0.01f, upVec);
	glm::mat4 yRot = glm::rotate( -a_offset.y * a_deltaTime * 0.01f, rightVec );
	
	if(glm::length(a_offset) > 0.01f)
		SetWorldTransform(GetWorldTransform() * (xRot * yRot));

	//warping occuring here because they are no longer normalized or perpendicular (orthonormal).
	//z axis is the one we want to keep the same
	//the right vector should then be calculated by crossing the worlds up and the z.
	//the camera up vector should then be calculated by crossing the right and z.
	//remember that the order matters with these- I may have it wrong.
	rightVec = glm::cross( m_up, (vec3)(GetWorldTransform()[2]));
	upVec = glm::cross((vec3)(GetWorldTransform()[2]), rightVec);	

	SetZRotation((vec3)glm::normalize(GetWorldTransform()[2]));
	rightVec = glm::normalize(rightVec);
	upVec = glm::normalize(upVec);

	SetXRotation(rightVec);
	SetYRotation(upVec);
}