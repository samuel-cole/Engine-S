#include "LevelData.h"
#include "PhysicsGame.h"

char LevelData::LoadLevel(PhysicsGame* a_game, int a_level, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes)
{
	switch (a_level)
	{
	case 0:
		return Level0(a_game, a_goalObject, a_targetShape, a_hazardShapes);
	case 1:
		return Level1(a_game, a_goalObject, a_targetShape, a_hazardShapes);
	case 2:
		return Level2(a_game, a_goalObject, a_targetShape, a_hazardShapes);
	case 3:
		return Level3(a_game, a_goalObject, a_targetShape, a_hazardShapes);
	case 4:
		printf("Congratulations, you completed the game!");
		return (char)-1;
	default:
		printf("Error: invalid level!");
		return (char)0;
	}
}

char LevelData::Level0(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes)
{
	unsigned int tetherPoints[2] = { 0, 19 };
	a_game->AddCloth(20, 2, tetherPoints, 10.0f);
	
	for (int i = -1; i < 2; ++i)
	{
		for (int j = -1; j < 2; ++j)
		{
			unsigned int newBox = a_game->AddBox(vec3(i * 5.0f, 20.0f, j * 5.0f), quat(vec3(30, 25, 70)));
			if (i == 0 && j == 0)
			{
				a_goalObject = newBox;
			}
		}
	}
	//a_goalObject = a_game->AddBox(vec3(0, 20.0f, 0), quat(vec3(30, 25, 70)));
	a_targetShape = a_game->AddStaticSphere(1.0f, vec3(-25.0f, 1.0f, -25.0f), true);
	a_hazardShapes.push_back(a_game->AddStaticSphere(1.0f, vec3(25.0f, 1.0f, 25.0f), true));
	
	a_game->AddFluid(vec3(-10, 0, -10), 20, 20, 20, 5);

	return (1 << PhysicsGame::GRAVITY) | (1 << PhysicsGame::RESTITUTION) | (1 << PhysicsGame::BOUYANCY);
}

char LevelData::Level1(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes)
{
	a_goalObject = a_game->AddBox(vec3(0.0f, 10, 0.0f), quat(vec3(30, 25, 70)));

	a_targetShape = a_game->AddStaticSphere(1.0f, vec3(0.0f, 5.0f, 0.0f), true);

	return (1 << PhysicsGame::GRAVITY) | (1 << PhysicsGame::BOUYANCY) | (1 << PhysicsGame::PAUSE_GAME);
}

char LevelData::Level2(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes)
{
	a_goalObject = a_game->AddBox(vec3(0.0f, 10, 0.0f), quat(vec3(30, 25, 70)));

	a_hazardShapes.push_back(a_game->AddStaticSphere(2.0f, vec3(0.0f, 5.0f, 0.0f), true));
	a_hazardShapes.push_back(a_game->AddStaticSphere(2.0f, vec3(-5.0f, 10.0f, 0.0f), true));
	a_hazardShapes.push_back(a_game->AddStaticSphere(2.0f, vec3(5.0f, 10.0f, 0.0f), true));
	a_hazardShapes.push_back(a_game->AddStaticSphere(2.0f, vec3(0.0f, 10.0f, -5.0f), true));
	a_hazardShapes.push_back(a_game->AddStaticSphere(2.0f, vec3(0.0f, 10.0f, 5.0f), true));

	a_targetShape = a_game->AddStaticSphere(1.0f, vec3(-25.0f, 1.0f, -25.0f), true);

	return (1 << PhysicsGame::GRAVITY);
}

char LevelData::Level3(PhysicsGame* a_game, unsigned int& a_goalObject, unsigned int& a_targetShape, std::vector<unsigned int>& a_hazardShapes)
{
	a_goalObject = -2;

	a_game->AddBox(vec3(0.0f, 10, 0.0f), quat(vec3(30, 25, 70)));

	a_game->AddFluid(vec3(-10, 0, -10), 20, 20, 20, 0);

	a_targetShape = a_game->AddStaticSphere(1.0f, vec3(0.0f, 30.0f, 0.0f), true);

	return (1 << PhysicsGame::GRAVITY) | (1 << PhysicsGame::BOUYANCY) | (1 << PhysicsGame::PAUSE_GAME);
}

