#ifndef TRACKER_CAMERA_H
#define TRACKER_CAMERA_H

#include "Camera.h"

class Renderer;

//=======================================================
//FleX project addition
//=======================================================
//Type of camera that tracks a render object.
class TrackerCamera : public Camera
{
private:
	//The render handle of the object to track.
	unsigned int m_renderHandle;
	//The renderer that contains the tracked object.
	Renderer* m_renderer;
public:
	//Sets the object for this camera to track to the object passed in as an argument.
	inline void SetObjectToTrack(const unsigned int a_renderHandle, Renderer* a_renderer)
	{
		m_renderHandle = a_renderHandle;
		m_renderer = a_renderer;
	}

	//Rotates the camera to face the tracked object.
	void Update(const float a_deltaTime);
};

#endif