This document describes the features within my program:

Setup Instructions for Running the Game:
Just run the Engine S.exe! It can be found in the 'Release' folder within the project folder.
Please note that the program requires a mid to high tier NVIDIA GPU in order to work properly.

Controls:
The game is controlled through the options bar- clicking the plus and minus buttons next to a number allows the value to be changed,
while clicking the circle allows the value to be modified by dragging the cursor in a circle.
The gravity arrow can also be dragged to different directions.
The pause/unpause time feature of the game can also be controlled via keyboard- the spacebar can be used to toggle this.

Goal:
The goal of the game is to move the blue goal object to the green target area.

Setup Instructions for Building the Game:
Open the Engine S.sln solution within the project folder with Visual Studio 2013, and run the project. 
All dependencies are included within the project folder, so no additional setup should be required.
Note that running in the debug configuration can cause massive performance issues on anything but the most powerful of GPUs.

Coding Standards:
See the 'CodeStandards.txt' file in the same folder as this document for a summary of the coding standards used for this project.


Crediting:
	Art:
		High-poly sphere model provided by AIE.

		Low poly sphere models from the NTU 3D Model Database, http://3d.csie.ntu.edu.tw/~dynamic/database/index.html
		Ding-Yun Chen, Xiao-Pei Tian, Yu-Te Shen and Ming Ouhyoung, "On Visual Similarity Based 3D Model Retrieval", Computer Graphics Forum (EUROGRAPHICS'03), Vol. 22, No. 3, pp. 223-232, Sept. 2003. 

	Code:
		FleX by NVIDIA (physics)
		OpenGL by Khronos Group. (rendering)
		OpenGL Mathematics (glm) by G-Truc Creation. (maths)
		GLFW by Camillia Berglund. (window handling)
		FBX Loader provided by AIE. (loading fbx files)
		AntTweakBar by Philippe Decaudin. (GUI)
		stb library by Sean Barrett. (loading images)
		
Everything else by me (Samuel Cole)!



Puzzle Walkthrough:

Before reading these, consider attempting the puzzles first- they're much more enjoyable without knowing the solutions! 
These should be used as a guide for if you are stuck and want to experience the rest of the game.

Level 1: Move the gravity arrow to move the object to the target shape (away from the camera and to the left).

Level 2: Unpause the game.

Level 3: Pause the game, and point the gravity arrow towards the camera. 
Once the cube has escaped the area bounded by the hazard objects,  move the gravity arrow towards the target shape (away from the camera and to the left).

Level 4: Unpause the game, then move the gravity arrow upwards.

Level 5: Move the gravity arrow towards the camera and to the right to move the fluid to the corner of the level. 
Once the fluid is in the corner, turn up the fluid cohesion value to collect the fluid into a single ball.
Now navigate this fluid past the hazard objects in the room to the goal object. (this is much easier with very low gravity strength or a low fluid gravity scale)