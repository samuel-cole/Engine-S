#ifndef FLY_CAMERA_H
#define FLY_CAMERA_H

#include "WalkCamera.h"
#include "AntTweakBar.h"

//=======================================================
//FleX project addition
//=======================================================
//Debug class for moving a camera through 3d space.
class FlyCamera : public WalkCamera
{
public:
	FlyCamera(TwBar* a_debugBar);

	//Moves the camera in the direction specified by the user's input.
	void Update(const float deltaTime) override;
};

#endif