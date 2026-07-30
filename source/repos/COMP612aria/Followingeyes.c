/******************************************************************************
 * Following Eyes
 * Two eyes whose pupils follow the mouse cursor while it's over the window.
 ******************************************************************************/

#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <stdio.h>

 /******************************************************************************
  * Animation & Timing Setup
  ******************************************************************************/

#define TARGET_FPS 60
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;
unsigned int frameStartTime = 0;

/******************************************************************************
 * Keyboard Input Handling Setup
 ******************************************************************************/

#define KEY_EXIT 27 // Escape key.
int renderFillEnabled = 1;

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void idle(void);
void mouseMoved(int x, int y);

/******************************************************************************
 * Animation-Specific Function Prototypes
 ******************************************************************************/

void main(int argc, char** argv);
void init(void);
void think(void);
void drawCircle(float cx, float cy, float radius, int segments);
void drawEye(float cx, float cy);

/******************************************************************************
 * Animation-Specific Setup
 ******************************************************************************/

int windowWidth = 800;
int windowHeight = 800;

#define EYE_RADIUS      90.0f
#define PUPIL_RADIUS    30.0f
#define PUPIL_MARGIN    12.0f
#define MAX_PUPIL_OFFSET (EYE_RADIUS - PUPIL_RADIUS - PUPIL_MARGIN)

const float leftEyeX = 300.0f;
const float rightEyeX = 500.0f;
const float eyeY = 400.0f;

// Latest mouse position in world coordinates (set by mouseMoved, used by think()).
float mouseWorldX = 400.0f;
float mouseWorldY = 400.0f;

// Current pupil offsets from their eye centres (set by think()).
float leftPupilOffsetX = 0.0f, leftPupilOffsetY = 0.0f;
float rightPupilOffsetX = 0.0f, rightPupilOffsetY = 0.0f;

/******************************************************************************
 * Entry Point
 ******************************************************************************/

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Following Eyes");

	init();

	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutIdleFunc(idle);
	glutPassiveMotionFunc(mouseMoved);

	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

	glutMainLoop();
}

/******************************************************************************
 * GLUT Callbacks
 ******************************************************************************/

void display(void)
{
	if (!renderFillEnabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawEye(leftEyeX, eyeY);
	drawEye(rightEyeX, eyeY);

	glutSwapBuffers();
}

void reshape(int width, int h)
{
	windowWidth = width;
	windowHeight = h;

	glViewport(0, 0, width, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, h, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {
	case 'l':
		renderFillEnabled = !renderFillEnabled;
		break;
	case KEY_EXIT:
		exit(0);
		break;
	}
}

/*
	GLUT reports mouse position with (0,0) at the top-left and y increasing
	downwards; we convert it to match our bottom-left-origin world coordinates.
*/
void mouseMoved(int x, int y)
{
	mouseWorldX = (float)x;
	mouseWorldY = (float)(windowHeight - y);
}

void idle(void)
{
	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) - frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}

	frameStartTime = glutGet(GLUT_ELAPSED_TIME);

	think();

	glutPostRedisplay();
}

/******************************************************************************
 * Animation-Specific Functions
 ******************************************************************************/

void init(void)
{
	// Set a purple background color.
	glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
}

/*
	Points each pupil towards the mouse, clamping the offset so it never
	leaves the eye.
*/
void think(void)
{
	float dx, dy, dist;

	dx = mouseWorldX - leftEyeX;
	dy = mouseWorldY - eyeY;
	dist = sqrtf(dx * dx + dy * dy);
	if (dist > MAX_PUPIL_OFFSET && dist > 0.0f)
	{
		dx = dx / dist * MAX_PUPIL_OFFSET;
		dy = dy / dist * MAX_PUPIL_OFFSET;
	}
	leftPupilOffsetX = dx;
	leftPupilOffsetY = dy;

	dx = mouseWorldX - rightEyeX;
	dy = mouseWorldY - eyeY;
	dist = sqrtf(dx * dx + dy * dy);
	if (dist > MAX_PUPIL_OFFSET && dist > 0.0f)
	{
		dx = dx / dist * MAX_PUPIL_OFFSET;
		dy = dy / dist * MAX_PUPIL_OFFSET;
	}
	rightPupilOffsetX = dx;
	rightPupilOffsetY = dy;
}

// Draws a filled circle at the given centre coordinates (cx and cy) with the given radius and number of segments.
// The circle is drawn as a triangle fan, with the centre vertex at (cx, cy) and the outer vertices evenly spaced around the circumference.
// The number of segments determines the smoothness of the circle; more segments result in a smoother circle.
// The circle is drawn in the current color, which can be set using glColor3f() before calling this function.
void drawCircle(float cx, float cy, float radius, int segments)
{
	int i;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(cx, cy);
	for (i = 0; i <= segments; i++)
	{
		float angle = 2.0f * 3.1415926f * (float)i / (float)segments;
		glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
	}
	glEnd();
}

// Draws an eye at the given centre coordinates (cx and cy), 
// with the pupil offset according to the current mouse position.
// The pupil offset is clamped so it never leaves the eye.
// The eye is drawn as a white circle with a black outline, and the pupil is drawn as a dark circle.
void drawEye(float cx, float cy) 
{
	float pupilOffsetX = (cx == leftEyeX) ? leftPupilOffsetX : rightPupilOffsetX;
	float pupilOffsetY = (cx == leftEyeX) ? leftPupilOffsetY : rightPupilOffsetY;

	glColor3f(1.0f, 1.0f, 1.0f);
	drawCircle(cx, cy, EYE_RADIUS, 60);

	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 60; i++)
	{
		float angle = 2.0f * 3.1415926f * (float)i / 60.0f;
		glVertex2f(cx + cosf(angle) * EYE_RADIUS, cy + sinf(angle) * EYE_RADIUS);
	}
	glEnd();

	glColor3f(0.05f, 0.05f, 0.05f);
	drawCircle(cx + pupilOffsetX, cy + pupilOffsetY, PUPIL_RADIUS, 40);
}