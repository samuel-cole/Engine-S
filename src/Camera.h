#ifndef CAMERA_H
#define CAMERA_H

#include "glm\glm.hpp"

using glm::mat4;
using glm::vec3;

class Camera
{
private:
	//The camera's location within the world.
	mat4 m_worldTransform;
	//View is the inverse of the world transform.
	mat4 m_viewTransform;
	//The transform required to move things into the projection position- takes into account the field of view and aspect ratio.
	mat4 m_projectionTransform;
	//The projection matrix multiplied by the view matrix- used to get objects into projection space.
	mat4 m_projectionViewTransform;

	void UpdateProjectionViewTransform();

public:
	virtual void Update( float a_deltaTime ) = 0;

	//Mutator functions
	//Sets the perspective matrix to have the field of view, aspect ration, and near and far planes passed in as arguments.
	void SetPerspective( float a_fieldOfView, float a_aspectRatio, float a_near, float a_far );
	//Sets the camera to look at a position- takes as arguments the position the camera should be at, the position the camera should be facing and a vector in the up direction.
	void SetLookAt( vec3 a_from, vec3 a_to, vec3 a_up );
	//Sets the position of the camera.
	void SetPosition( vec3 a_position );
	//Sets the world transform.
	void SetWorldTransform(mat4 a_worldTransform);


	//Rotation setting functions.
	//Sets the X rotation.
	void SetXRotation( vec3 a_rotation );
	//Sets the Y rotation.
	void SetYRotation( vec3 a_rotation );
	//Sets the Z rotation.
	void SetZRotation( vec3 a_rotation );

	//Accessor functions
	//Returns the world transform matrix.
	inline mat4 GetWorldTransform()
	{
		return m_worldTransform;
	}
	//Returns the view transform matrix.
	inline mat4 GetView()
	{
		return m_viewTransform;
	}
	//Returns the projection matrix.
	inline mat4 GetProjection()
	{
		return m_projectionTransform;
	}
	//Returns the projection-view maxtrix.
	inline mat4 GetProjectionView()
	{
		return m_projectionViewTransform;
	}
};

#endif