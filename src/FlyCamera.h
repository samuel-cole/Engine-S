#ifndef FLY_CAMERA_H
#define FLY_CAMERA_H

#include "Camera.h"
#include "AntTweakBar.h"

class FlyCamera: public Camera
{
public:
	FlyCamera(TwBar* a_debugBar);

	void Update (float a_deltaTime);

	inline void SetSpeed(float a_speed)
	{
		m_speed = a_speed;
	}
private:
	float m_speed;
	vec3 m_up;
	bool m_viewButtonClicked;
	glm::vec2 m_mouseStartPos;

	void SetRotationFromMouseOffset(glm::vec2 a_offset, float a_deltaTime);
};

#endif