#include <vgl.h>
#include <stdio.h>
#include <InitShader.h>
#include <vec.h>

#include "MyPlane.h"

MyPlane plane;

float g_rotTime = 0.0f;
float g_waveTime = 0.0f;
bool g_bPlay = false;//start rot and waving
bool g_bWavePlay = false;//show wave
int divisions = 30;

void myInit() {
	plane.init(divisions);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//enable depth buffer
	glEnable(GL_DEPTH_TEST);//enable depth buffer

	plane.draw(g_rotTime, g_bWavePlay);

	glFlush();

	glutSwapBuffers();
}

void myIdle() {
	Sleep(16);

	if (g_bPlay)
	{
		g_rotTime += 0.016f;
	}
	glutPostRedisplay();
	
}

void myKeyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case ' ': g_bPlay = !g_bPlay;	break;
	case 'w': g_bWavePlay = !g_bWavePlay;	break;
	case '1': // decrease division
		if (divisions > 2) {
			divisions--;
			plane.init(divisions);   //init again
		}
		break;

	case '2': // increase division
		divisions++;
		plane.init(divisions);       //init again
		break;

	case 'q': exit(0);		break;

	default:								break;
	}

}
void startIntroduction() {
	printf("21011794 HJD, \"A Waving Plane\"\n");
	printf("Assignment #1 for Computer Graphics\n");
	printf("------------------------------------------------------------\n");
	printf("'1' key: Decreasing the Number of Division\n");
	printf("'2' key: Increaing the Number of Division\n");
	printf("'w' key : Showing / hiding the waving pattern\n");
	printf("Spacebar : starting / stoping rotating and waving\n\n");
	printf("`Q' Key: Exit the program.\n");
	printf("------------------------------------------------------------\n\n");
}
int main(int argc, char** argv)
{
	startIntroduction();
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);//enable depth
	glutInitWindowSize(512, 512);
	glutCreateWindow("Hello GL");

	glewExperimental = true;
	glewInit();
	

	myInit();
	glutDisplayFunc(display);
	glutIdleFunc(myIdle);
	glutKeyboardFunc(myKeyboard);


	glutMainLoop();
	return 0;
}