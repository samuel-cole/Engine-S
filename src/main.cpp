
#include "checkers\checkers.h"


int main ()
{
	Checkers* game = new Checkers;

	int returnValue = game->Run();
	delete game;

	return returnValue;
}