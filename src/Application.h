#ifndef APPLICATION_H
#define APPLICATION_H

#include "AntTweakBar.h"

struct GLFWwindow;


//Base application class. Contains logic for the update loop and initialising the window.
class Application
{
public:
	//Function to be called by main- starts the game, and then runs the main update loop.
	virtual int Run();
protected:
	//Called when the game starts. Handles setting up the window- implementations of the application class should call this function within their Init() function.
	virtual int Init();
	//Called when the game ends. Handles cleaning up the window- implementations of the application class should call this function within their Deinit() function.
	virtual int Deinit();
	//Called every frame before Draw()- update logic should go here.
	virtual void Update(float a_deltaTime) = 0;
	//Called every frame after Update()- draw logic should go here.
	virtual void Draw() = 0;

	//Pointer to the window the game is running in.
	GLFWwindow* m_window;

	//Pointer to the anttweakbar used in this game.
	TwBar* m_debugBar;

	//The current time.
	float m_currentTime;
	//The amount of time since the last frame.
	float m_deltaTime;
	//The amount of time during the previous frame.
	float m_previousTime;
};

#endif