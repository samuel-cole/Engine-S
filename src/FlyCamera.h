#ifndef FLY_CAMERA_H
#define FLY_CAMERA_H

#include "WalkCamera.h"
#include "AntTweakBar.h"

//=======================================================
//FleX project addition
//=======================================================
//Class that handles camera movement.
class FlyCamera : public WalkCamera
{
public:
	FlyCamera(TwBar* a_debugBar);

	void Update(const float deltaTime) override;
};

#endif