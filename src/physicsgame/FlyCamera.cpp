#include "FlyCamera.h"
#include "InputManager.h"

FlyCamera::FlyCamera(TwBar* a_debugBar) : WalkCamera(a_debugBar)
{}

void FlyCamera::Update(const float deltaTime)
{
	WalkCamera::Update(deltaTime);

	if (InputManager::GetKey(Keys::W) || InputManager::GetKey(Keys::UP))
	{
		mat4 world = GetWorldTransform();
		SetPosition((vec3)(world[3]) - vec3(world[2].x, world[2].y, world[2].z));
	}
	if (InputManager::GetKey(Keys::S) || InputManager::GetKey(Keys::DOWN))
	{
		mat4 world = GetWorldTransform();
		SetPosition((vec3)(world[3]) + vec3(world[2].x, world[2].y, world[2].z));
	}
	if (InputManager::GetKey(Keys::A) || InputManager::GetKey(Keys::LEFT))
	{
		mat4 world = GetWorldTransform();
		SetPosition((vec3)(world[3]) - vec3(world[0].x, world[0].y, world[0].z));
	}
	if (InputManager::GetKey(Keys::D) || InputManager::GetKey(Keys::RIGHT))
	{
		mat4 world = GetWorldTransform();
		SetPosition((vec3)(world[3]) + vec3(world[0].x, world[0].y, world[0].z));
	}
}