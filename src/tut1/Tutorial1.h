#ifndef TUTORIAL_1
#define TUTORIAL_1

#include "Application.h"
#include "glm\glm.hpp"

class Tutorial1 : public Application
{
private:
	int Init();
	int Deinit();
	void GameLoop();

	glm::mat4 view;
	glm::mat4 projection;
};

#endif