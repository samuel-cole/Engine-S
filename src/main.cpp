
#include "checkers\checkers.h"
#include "physicsgame\TestScene.h"

int main ()
{
	//Checkers* game = new Checkers;
	TestScene* game = new TestScene;
	int returnValue = game->Run();
	delete game;

	return returnValue;
}