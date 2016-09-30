//#include "physicsgame\TestScene.h"
#include "physicsgame\PhysicsGame.h"

int main ()
{
	//TestScene* game = new TestScene;
	PhysicsGame* game = new PhysicsGame;
	int returnValue = game->Run();
	delete game;

	return returnValue;
}