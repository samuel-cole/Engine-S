#include "tut13\Tutorial13.h"
#include "checkersTest\CheckersTest1.h"
#include "physX\Physics2.h"

int main ()
{
	//MirrorTest1* game = new MirrorTest1;
	//CheckersTest* game = new CheckersTest;
	//Tutorial13* game = new Tutorial13;
	Physics2* game = new Physics2;

	return game->Run();
}