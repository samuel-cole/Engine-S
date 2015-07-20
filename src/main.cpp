#include "tut13\Tutorial13.h"
#include "checkersTest\CheckersTest2.h"
#include "physX\PhysicsConcrete1.h"

int main ()
{
	//MirrorTest1* game = new MirrorTest1;
	CheckersTest2* game = new CheckersTest2;
	//Tutorial13* game = new Tutorial13;
	//PhysicsConcrete1* game = new PhysicsConcrete1;

	return game->Run();
}