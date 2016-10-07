#ifndef TRACKER_CAMERA_H
#define TRACKER_CAMERA_H

#include "Camera.h"

class Renderer;

//Type of camera that tracks a render object.
class TrackerCamera : public Camera
{
private:
	unsigned int m_renderHandle;
	Renderer* m_renderer;
public:
	inline void SetObjectToTrack(const unsigned int a_renderHandle, Renderer* a_renderer)
	{
		m_renderHandle = a_renderHandle;
		m_renderer = a_renderer;
	}

	void Update(const float a_deltaTime);
};

#endif