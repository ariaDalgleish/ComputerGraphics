/************************************************************************************

	File: 			rotateSquareDouble.c

	Description:	A complete OpenGL program to draw a rotating square on the screen.
					Uses double buffering.

	Author:			Angel, modified by Jacqui Whalley

*************************************************************************************/


/* include the library header files */
#include <freeglut.h>
#include <math.h>

// the ratio of the circumference to the diameter of a circle  
#define PI 3.14159265

// conversion multiplier for converting from degrees to Radians
// NOTE: this is wrong in the "OpenGL: A Primer" book, pg. 45
#define DEG_TO_RAD PI/180.0

// angle of rotation
GLfloat theta = 0.0;


/************************************************************************

	Function:		initializeGL

	Description:	Initializes the OpenGL rendering context for display.

*************************************************************************/
void initializeGL(void)
{
	// set background color to be black
	glClearColor(0, 0, 0, 1.0);

	// set the drawing to be white
	glColor3f(1.0, 1.0, 1.0);

	// set window mode to 2D orthographic and set the window size 
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}


/************************************************************************

	Function:		myIdle

	Description:	Updates the animation when idle.

*************************************************************************/
void idle(void)
{

	// update the angle of rotation 
	theta += 0.02f;

	// if we have done a full turn, start at zero again
	if (theta >= 360.0)
		theta -= 360.0;

	// now force OpenGL to redraw the change
	glutPostRedisplay();
}



/************************************************************************

	Function:		myDisplay

	Description:	Displays a white square on a black background.

*************************************************************************/
void display(void)
{
	// clear the screen 
	glClear(GL_COLOR_BUFFER_BIT);

	// draw the square
	glBegin(GL_POLYGON);
	// need to convert to Radians since that is what sin and cos take as input
	glVertex2f((float)cos(DEG_TO_RAD * theta), (float)sin(DEG_TO_RAD * theta));
	glVertex2f((float)cos(DEG_TO_RAD * (theta + 90)), (float)sin(DEG_TO_RAD * (theta + 90)));
	glVertex2f((float)cos(DEG_TO_RAD * (theta + 180)), (float)sin(DEG_TO_RAD * (theta + 180)));
	glVertex2f((float)cos(DEG_TO_RAD * (theta + 270)), (float)sin(DEG_TO_RAD * (theta + 270)));
	glEnd();

	// swap the frame buffers
	glutSwapBuffers();
}

/************************************************************************

	Function:		main

	Description:	Sets up the openGL rendering context and the windowing
					system, then begins the display loop.

*************************************************************************/
void main(int argc, char** argv)
{
	// initialize the toolkit
	glutInit(&argc, argv);
	// set display mode: double buffering, color RGB
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	// set window size
	glutInitWindowSize(500, 500);
	// set window position on screen
	glutInitWindowPosition(100, 150);
	// open the screen window
	glutCreateWindow("rotate square double buffered");

	// register redraw function
	glutDisplayFunc(display);
	// register the idle function
	glutIdleFunc(idle);

	//initialize the rendering context
	initializeGL();
	// go into a perpetual loop
	glutMainLoop();
}