#ifndef WALK_CAMERA_H
#define WALK_CAMERA_H

#include "Camera.h"
#include "AntTweakBar.h"

//Class that handles camera rotations with a control scheme similar to those used in many modern 'first person' style games.
//WalkCamera is designed to be used with physX- the camera is moved with an object. As such, camera movement isn't handled by this class- only rotations are handled.
class WalkCamera : public Camera
{
public:
	WalkCamera(TwBar* a_debugBar);

	void Update(const float a_deltaTime) override;
	inline void SetPosition(const vec3& a_position)
	{
		mat4 world = GetWorldTransform();
		world[3] = glm::vec4(a_position, 1);
		SetWorldTransform(world);
	}
private:
	vec3 m_up;
	//Whether this frame should be checking mouse offset, or resetting it.
	int m_checkFrame;
	glm::vec2 m_mouseStartPos;

	void SetRotationFromMouseOffset(const glm::vec2& a_offset, float a_deltaTime);
};

#endif