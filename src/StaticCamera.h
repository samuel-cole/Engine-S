#ifndef STATIC_CAMERA_H
#define STATIC_CAMERA_H

#include "Camera.h"

class StaticCamera : public Camera
{
public:
	void Update(const float a_deltaTime);
};

#endif