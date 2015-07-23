#include "tut13\Tutorial13.h"
#include "checkersTest\CheckersTest2.h"
#include "physX\PhysicsConcrete2.h"

#include <vld.h>

int main ()
{
	//MirrorTest1* game = new MirrorTest1;
	CheckersTest2* game = new CheckersTest2;
	//Tutorial13* game = new Tutorial13;
	//PhysicsConcrete2* game = new PhysicsConcrete2;

	int returnValue = game->Run();
	delete game;
	return returnValue;
}