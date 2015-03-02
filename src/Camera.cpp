#include "Camera.h"
#include "glm\ext.hpp"

void Camera::SetPerspective( float a_fieldOfView, float a_aspectRatio, float a_near, float a_far )
{
	m_projectionTransform = glm::perspective(a_fieldOfView, a_aspectRatio, a_near, a_far);

	UpdateProjectionViewTransform();
}

void Camera::SetLookAt( vec3 a_from, vec3 a_to, vec3 a_up )
{
	//Get the view transform.
	m_viewTransform = glm::lookAt(a_from, a_to, a_up);
	//Inverse the view transform to get the world transform.
	m_worldTransform = glm::inverse(m_viewTransform);
	
	UpdateProjectionViewTransform();
}

void Camera::SetPosition( vec3 a_position )
{
	mat4 test = glm::inverse(m_worldTransform);

	m_worldTransform[3] = glm::vec4(a_position, 1);	

	UpdateProjectionViewTransform();
}

void Camera::SetXRotation( vec3 a_rotation )
{
	m_worldTransform[0] = glm::vec4(a_rotation, 0);

	UpdateProjectionViewTransform();
}

void Camera::SetYRotation( vec3 a_rotation )
{
	m_worldTransform[1] = glm::vec4(a_rotation, 0);

	UpdateProjectionViewTransform();
}

void Camera::SetZRotation( vec3 a_rotation )
{
	m_worldTransform[2] = glm::vec4(a_rotation, 0);

	UpdateProjectionViewTransform();
}

void Camera::SetWorldTransform (mat4 a_worldTransform)
{
	m_worldTransform = a_worldTransform;

	UpdateProjectionViewTransform();
}

void Camera::UpdateProjectionViewTransform()
{
	m_viewTransform = glm::inverse(m_worldTransform);
	m_projectionViewTransform = m_projectionTransform * m_viewTransform;
}