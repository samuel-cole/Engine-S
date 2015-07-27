#ifndef STATIC_CAMERA_H
#define STATIC_CAMERA_H

#include "Camera.h"

//Type of camera that doesn't move at all.
class StaticCamera : public Camera
{
public:
	void Update(const float a_deltaTime);
};

#endif