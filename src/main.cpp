#include "tut12\Tutorial12.h"
#include "checkersTest\CheckersTest1.h"

int main ()
{
	//MirrorTest1* game = new MirrorTest1;
	CheckersTest* game = new CheckersTest;

	return game->Run();
}