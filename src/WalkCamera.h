#ifndef WALK_CAMERA_H
#define WALK_CAMERA_H

#include "Camera.h"
#include "AntTweakBar.h"

//WalkCamera is designed to be used with physX- the camera is moved with an object. As such, camera movement isn't handled by this class- only rotations are handled.
class WalkCamera : public Camera
{
public:
	WalkCamera(TwBar* a_debugBar);

	void Update(const float a_deltaTime);
	inline void SetPosition(const vec3& a_position)
	{
		mat4 world = GetWorldTransform();
		world[3] = glm::vec4(a_position, 1);
		SetWorldTransform(world);
	}
private:
	vec3 m_up;
	bool m_viewButtonClicked;
	glm::vec2 m_mouseStartPos;

	void SetRotationFromMouseOffset(const glm::vec2& a_offset, float a_deltaTime);
};

#endif