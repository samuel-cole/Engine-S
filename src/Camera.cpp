#include "Camera.h"
#include "glm\ext.hpp"

void Camera::SetPerspective(const float a_fieldOfView, const float a_aspectRatio, const float a_near, const float a_far)
{
	m_projectionTransform = glm::perspective(a_fieldOfView, a_aspectRatio, a_near, a_far);

	UpdateProjectionViewTransform();
}

void Camera::SetPerspectiveOrtho(float a_left, float a_right, float a_top, float a_bottom, float a_near, float a_far)
{

	m_projectionTransform = glm::ortho<float>(a_left, a_right, a_top, a_bottom, a_near, a_far);
	UpdateProjectionViewTransform();
}

void Camera::SetLookAt(const vec3& a_from, const vec3& a_to, const vec3& a_up)
{
	//Get the view transform.
	m_viewTransform = glm::lookAt(a_from, a_to, a_up);
	//Inverse the view transform to get the world transform.
	m_worldTransform = glm::inverse(m_viewTransform);
	
	UpdateProjectionViewTransform();
}

void Camera::SetPosition(const vec3& a_position)
{
	mat4 test = glm::inverse(m_worldTransform);

	m_worldTransform[3] = glm::vec4(a_position, 1);	

	UpdateProjectionViewTransform();
}

void Camera::SetXRotation(const vec3& a_rotation)
{
	m_worldTransform[0] = glm::vec4(a_rotation, 0);

	UpdateProjectionViewTransform();
}

void Camera::SetYRotation(const vec3& a_rotation)
{
	m_worldTransform[1] = glm::vec4(a_rotation, 0);

	UpdateProjectionViewTransform();
}

void Camera::SetZRotation(const vec3& a_rotation)
{
	m_worldTransform[2] = glm::vec4(a_rotation, 0);

	UpdateProjectionViewTransform();
}

void Camera::SetWorldTransform (const mat4& a_worldTransform)
{
	m_worldTransform = a_worldTransform;

	UpdateProjectionViewTransform();
}

void Camera::UpdateProjectionViewTransform()
{
	m_viewTransform = glm::inverse(m_worldTransform);
	m_projectionViewTransform = m_projectionTransform * m_viewTransform;
}