#include <freeglut.h>

// global mouse variables
GLint mousePressed = 0;
GLfloat mouseX, mouseY;

// window size parameters
GLint windowWidth = 500;
GLint windowHeight = 400;



void initializeGL()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// set it to draw a big red dot
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(8.0);

	//ranges of x and y are from 0 to 1.0
	gluOrtho2D(0, 1.0, 0, 1.0);
}

void mouseButton(int button, int state, int x, int y)
{
	// if right button is pressed, exit the program
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		exit(0);
	}
	// if left button is pressed, record the mouse position and redraw
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mousePressed = 1; // store that the mouse is pressed
		mouseX = (GLfloat)x / (GLfloat)windowWidth;
		// convert x from Mouse to OpenGL coordinates
		//convert y from Mouse to OpenGL coordinates
		mouseY = (GLfloat)windowHeight - (GLfloat)y; // first invert mouse Y pos
		mouseY = mouseY / (GLfloat)windowHeight; // then convert to OpenGL coordinates

		glutPostRedisplay(); // now force a redraw
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT); // clear the window
	if (mousePressed)
	{
		glBegin(GL_POINTS);
		glVertex2f(mouseX, mouseY);
		glEnd();
	}
	glFlush();
}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(100, 150);
	glutCreateWindow("dots");

	//register redraw function
	glutDisplayFunc(display);
	//register mouse function
	glutMouseFunc(mouseButton);

	initializeGL();
	glutMainLoop();
}

