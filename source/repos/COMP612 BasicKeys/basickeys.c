#define _CRT_SECURE_NO_DEPRECATE

/*include library headers*/
#include <freeglut.h>
#include <math.h>

/*******************************************************************************
  * GLUT Callback Prototypes
  ******************************************************************************/

void display(void);
void keyPressed(unsigned char key, int x, int y);

/*******************************************************************************/

// original window size
int originalWidth = 500;
int originalHieght = 500;

//boolean flag to control drawing
int squareOn = 1;

// global size multiplier for the square
float shrinkAmount = 1.0;


void init()
{

	// set the clear color and the drawing color
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glColor3f(1.0, 1.0, 1.0);

	// set window mode to 2D orthographic and set the 2D coordinates 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

void drawSquare() {

	glBegin(GL_POLYGON);
	glVertex2f(0.5, 0.5);
	glVertex2f(-0.5, 0.5);
	glVertex2f(-0.5, -0.5);
	glVertex2f(0.5, -0.5);
	glEnd();

}

void display(void)
{
	// clear the screen 
	glClear(GL_COLOR_BUFFER_BIT);

	if (squareOn)
		drawSquare();

	glutSwapBuffers();
}

void keyPressed(unsigned char key, int x, int y)
{
	int sceneChanged = 0;

	switch (tolower(key)) {
	case 's':
		squareOn = !squareOn;
		sceneChanged = 1;
		break;
	case 'q':
		exit(0);
		break;
	}

	//comment this out - What happens? Why?
	if (sceneChanged) glutPostRedisplay();
}

void main(int argc, char** argv)
{
	// initialize the opengGL window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 150);
	glutCreateWindow("simple key input handling");

	//setup the scene
	init();

	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	//comment this out - What happens? Why?
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// register callbacks
	glutDisplayFunc(display);
	glutKeyboardFunc(keyPressed);

	// go into a perpetual loop
	glutMainLoop();
}


