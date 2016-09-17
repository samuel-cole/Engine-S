#ifndef TEST_SCENE_H
#define TEST_SCENE_H

#include "FleXBase.h"

class TestScene : public FleXBase
{
protected:
	virtual int Init() override;
	//virtual int Deinit() override;
	virtual void Update(float a_deltaTime) override;
	//virtual void Draw() override;

	vec3 m_gravityDir;
	float m_gravityStrength;

	vec3 m_oldGravityDir;
	float m_oldGravityStrength;

	void DebugUpdate(float a_deltaTime);
};

#endif