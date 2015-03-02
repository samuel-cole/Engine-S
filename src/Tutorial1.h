#ifndef TUTORIAL_1
#define TUTORIAL_1

#include "Application.h"
#include "glm\glm.hpp"

class Tutorial1 : public Application
{
private:
	int Init();
	int Deinit();
	void Update(float a_deltaTime);
	void Draw();

	glm::mat4 view;
	glm::mat4 projection;

public:
};

#endif