#include <iostream>
#include "CHIP8.h"
#include <glut.h>
#include <SDL_audio.h>

/*
CREDIT TO: LAURENCE MULLER
Most of the code from this file (using the OpenGL API) is taken from him.
Link to his tutorial and his code:
http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
I'm currently adding onto it by using SDL in order to use audio
*/
//modifier for the display when changing window size
int modifier = 10;

// Window size
int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;

void display();
void reshape_window(GLsizei w, GLsizei h);

// Use new drawing method
typedef unsigned __int8 u8;
u8 screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3];
void setupTexture();
void keypressed(unsigned char key, int x, int y);
void keyreleased(unsigned char key, int x, int y);

CHIP8system myChip8;


int main(int argc, char**argv)
{
	//If no file is given, first prompt the user to input in a file path
	if (argc==1)
	{
		char in;
		std::cout << "No file was given. Do you want to load in a game?(Y/N)" << std::endl;
		std::cout << "Typing anything else would close the program." << std::endl;
		std::cin >> in;

		if ((in == 'Y')||(in=='y'))
		{
		char filepath[500];
		std::cin.ignore();

		givefile:
			std::cout << "Please enter in the absolute file path you want to load in: " << std::endl;
			std::cin.getline(filepath, 500);
			if (*filepath == '\0')
			{
				goto givefile;
			}
			myChip8.loadgame(filepath);
		}
		else
		{
			goto end;
		}
	}
	else
	{
		myChip8.loadgame(argv[1]);
	}
	// Setup OpenGL
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(display_width, display_height);
	glutInitWindowPosition(320, 320);
	glutCreateWindow("DZNTS");

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape_window);
	//keyboard
	glutKeyboardFunc(keypressed);
	glutKeyboardUpFunc(keyreleased);
	//audio
	setupTexture();
	glutMainLoop();
	end:
	return 0;
}

// Setup Texture
void setupTexture()
{
	// Clear screen
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
		for (int x = 0; x < SCREEN_WIDTH; ++x)
			screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;

	// Create a texture 
	glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

	// Set up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Enable textures
	glEnable(GL_TEXTURE_2D);
}

void updateTexture(const CHIP8system& c8)
{
	// Update pixels
	for (int y = 0; y < 32; ++y)
		for (int x = 0; x < 64; ++x)
			if (c8.display[(y * 64) + x] == 0)
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;	// Disabled
			else
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 255;  // Enabled

	// Update Texture
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);		glVertex2d(0.0, 0.0);
	glTexCoord2d(1.0, 0.0); 	glVertex2d(display_width, 0.0);
	glTexCoord2d(1.0, 1.0); 	glVertex2d(display_width, display_height);
	glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0, display_height);
	glEnd();
}

void display()
{
	myChip8.emulatecycle();

	if (myChip8.drawflag)
	{
		// Clear framebuffer
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw pixels to texture
		updateTexture(myChip8);

		// Swap buffers!
		glutSwapBuffers();

		// Processed frame
		myChip8.drawflag = false;
	}
}

void reshape_window(GLsizei w, GLsizei h)
{
	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, h, 0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);

	// Resize quad
	display_width = w;
	display_height = h;
}

void keypressed(unsigned char key,int x, int y)
{
	if (key == 8)//backspace key pressed
	{
		exit(0);
	}
	else if (key == '1') myChip8.key[1]=1;
	else if (key == '2') myChip8.key[2]=1;
	else if (key == '3') myChip8.key[3]=1;
	else if (key == '4') myChip8.key[12]=1;
	else if (key == 'q') myChip8.key[4]=1;
	else if (key == 'w') myChip8.key[5] = 1;
	else if (key == 'e') myChip8.key[6]=1;
	else if (key == 'r') myChip8.key[13]=1;
	else if (key == 'a') myChip8.key[7]=1;
	else if (key == 's') myChip8.key[8]=1;
	else if (key == 'd') myChip8.key[9]=1;
	else if (key == 'f') myChip8.key[14]=1;
	else if (key == 'z') myChip8.key[10]=1;
	else if (key == 'x') myChip8.key[0]=1;
	else if (key == 'c') myChip8.key[11]=1;
	else if (key == 'v') myChip8.key[15]= 1;
}

void keyreleased(unsigned char key, int x, int y)
{
	if (key == '1') myChip8.key[1] = 0;
	else if (key == '2') myChip8.key[2] = 0;
	else if (key == '3') myChip8.key[3] = 0;
	else if (key == '4') myChip8.key[12] = 0;
	else if (key == 'q') myChip8.key[4] = 0;
	else if (key == 'w') myChip8.key[5] = 0;
	else if (key == 'e') myChip8.key[6] = 0;
	else if (key == 'r') myChip8.key[13] = 0;
	else if (key == 'a') myChip8.key[7] = 0;
	else if (key == 's') myChip8.key[8] = 0;
	else if (key == 'd') myChip8.key[9] = 0;
	else if (key == 'f') myChip8.key[14] = 0;
	else if (key == 'z') myChip8.key[10] = 0;
	else if (key == 'x') myChip8.key[0] = 0;
	else if (key == 'c') myChip8.key[11]= 0;
	else if (key == 'v') myChip8.key[15]= 0;
}

