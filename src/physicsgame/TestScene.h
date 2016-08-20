#ifndef TESTSCENE_H
#define TESTSCENE_H

#include "Application.h"

class WalkCamera;
class Renderer;
struct FlexSolver;

//struct FlexParticle
//{
//	vec3 position;
//	vec3 velocity;
//	float inverseMass;
//	int phase;
//}

class TestScene : public Application
{
protected:
	virtual int Init() override;
	virtual int Deinit() override;
	virtual void Update(float a_deltaTime) override;
	virtual void Draw() override;

	//The camera used within the scene.
	WalkCamera* m_camera;
	//The renderer used to render the scene.
	Renderer* m_renderer;

	FlexSolver* m_solver;

	void AddCloth(unsigned int a_dimensions);

	unsigned int m_clothModel;

	float m_particleRadius;
};



#endif