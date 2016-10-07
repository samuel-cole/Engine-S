#include "TrackerCamera.h"
#include "Renderer.h"

void TrackerCamera::Update(const float a_deltaTime)
{
	if (m_renderer)
	{
		SetLookAt((vec3)(GetWorldTransform()[3]), m_renderer->GetPosition(m_renderHandle), vec3(0, 1, 0));
	}
}