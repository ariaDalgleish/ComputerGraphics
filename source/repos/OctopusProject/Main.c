/******************************************************************************
 *
 * Computer Graphics Programming 2026 Project Template v2.3 (05/03/2026)
 *
 * Based on: Animation Controller v1.0 (11/04/2021)
 *
 * This template provides a basic FPS-limited render loop for an animated scene,
 * plus keyboard handling for smooth game-like control of an object such as a
 * character or vehicle.
 *
 * A simple static lighting setup is provided via initLights(), which is not
 * included in the animationalcontrol.c template. There are no other changes.
 *
 ******************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE  

#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <stdlib.h>  
#include <stdio.h> 
#include <ctype.h>

 /******************************************************************************
  * Animation & Timing Setup
  ******************************************************************************/

  // Target frame rate (number of Frames Per Second).
#define TARGET_FPS 60				

// Ideal time each frame should be displayed for (in milliseconds).
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;

// Frame time in fractional seconds.
// Note: This is calculated to accurately reflect the truncated integer value of
// FRAME_TIME, which is used for timing, rather than the more accurate fractional
// value we'd get if we simply calculated "FRAME_TIME_SEC = 1.0f / TARGET_FPS".
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;

// Time we started preparing the current frame (in milliseconds since GLUT was initialized).
unsigned int frameStartTime = 0;

/******************************************************************************
 * Some Simple Definitions of Motion
 ******************************************************************************/

#define MOTION_NONE 0				// No motion.
#define MOTION_CLOCKWISE -1			// Clockwise rotation.
#define MOTION_ANTICLOCKWISE 1		// Anticlockwise rotation.
#define MOTION_BACKWARD -1			// Backward motion.
#define MOTION_FORWARD 1			// Forward motion.
#define MOTION_LEFT -1				// Leftward motion.
#define MOTION_RIGHT 1				// Rightward motion.
#define MOTION_DOWN -1				// Downward motion.
#define MOTION_UP 1					// Upward motion.

 // Represents the motion of an object on four axes (Yaw, Surge, Sway, and Heave).
 // 
 // You can use any numeric values, as specified in the comments for each axis. However,
 // the MOTION_ definitions offer an easy way to define a "unit" movement without using
 // magic numbers (e.g. instead of setting Surge = 1, you can set Surge = MOTION_FORWARD).
 //
typedef struct {
	int Yaw;		// Turn about the Z axis	[<0 = Clockwise, 0 = Stop, >0 = Anticlockwise]
	int Surge;		// Move forward or back		[<0 = Backward,	0 = Stop, >0 = Forward]
	int Sway;		// Move sideways (strafe)	[<0 = Left, 0 = Stop, >0 = Right]
	int Heave;		// Move vertically			[<0 = Down, 0 = Stop, >0 = Up]
} motionstate4_t;

/******************************************************************************
 * Keyboard Input Handling Setup
 ******************************************************************************/

 // Represents the state of a single keyboard key.Represents the state of a single keyboard key.
typedef enum {
	KEYSTATE_UP = 0,	// Key is not pressed.
	KEYSTATE_DOWN		// Key is pressed down.
} keystate_t;

// Represents the states of a set of keys used to control an object's motion.
typedef struct {
	keystate_t MoveForward;
	keystate_t MoveBackward;
	keystate_t MoveLeft;
	keystate_t MoveRight;
	keystate_t MoveUp;
	keystate_t MoveDown;
	keystate_t TurnLeft;
	keystate_t TurnRight;
} motionkeys_t;

// Current state of all keys used to control our "player-controlled" object's motion.
motionkeys_t motionKeyStates = {
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP,
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP };

// How our "player-controlled" object should currently be moving, solely based on keyboard input.
//
// Note: this may not represent the actual motion of our object, which could be subject to
// other controls (e.g. mouse input) or other simulated forces (e.g. gravity).
motionstate4_t keyboardMotion = { MOTION_NONE, MOTION_NONE, MOTION_NONE, MOTION_NONE };

// Define all character keys used for input (add any new key definitions here).
// Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
// characters typed by the user to lowercase, so the SHIFT key is ignored.

#define KEY_MOVE_FORWARD	'w'
#define KEY_MOVE_BACKWARD	's'
#define KEY_MOVE_LEFT		'a'
#define KEY_MOVE_RIGHT		'd'
#define KEY_RENDER_FILL		'l'
#define KEY_EXIT			27 // Escape key.
#define KEY_TOGGLE_BUBBLES	'b'
#define KEY_TOGGLE_PROPELLER ' '   // spacebar

// Define all GLUT special keys used for input (add any new key definitions here).

#define SP_KEY_MOVE_UP		GLUT_KEY_UP
#define SP_KEY_MOVE_DOWN	GLUT_KEY_DOWN
#define SP_KEY_TURN_LEFT	GLUT_KEY_LEFT
#define SP_KEY_TURN_RIGHT	GLUT_KEY_RIGHT

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void specialKeyPressed(int key, int x, int y);
void keyReleased(unsigned char key, int x, int y);
void specialKeyReleased(int key, int x, int y);
void idle(void);

/******************************************************************************
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/

void main(int argc, char** argv);
void init(void);
void think(void);
void initLights(void);

void drawOctopus(void);
void drawOriginMarker(void);
void drawTentacle(void);
void drawTentacleRing(void);

void initSeaweed(void);
void createSeaweedDisplayList(void);
void drawSeaweedStrand(void);
void drawSeaweed(void);

void createSeabedTexture(void);
void createSeabedDisplayList(void);
void drawSeabed(void);
void createSeabedDisplayList(void);

void initPodLights(void);
void updatePodLights(void);
void setupPodLights(void);
void drawPod(void);
void drawPoster(void);
void createPosterTexture(void);
void drawPodBeams(void);
void updateFog(double eyeX, double eyeY, double eyeZ);

/******************************************************************************
 * Animation-Specific Setup (Add your own definitions, constants, and globals here)
 ******************************************************************************/

 //  SCALE: mapping of real-world units to OpenGL units
 //  (1 GL unit = 1 m)
#define BODY_RADIUS 1.0
#define OCTOPUS_COLLIDE_RADIUS 0.6f   // About the octopus's body radius (0.5 m) plus a little.
#define EYE_RADIUS 0.1
#define BEAK_SCALE_FACTOR BODY_RADIUS/3.0
#define TO_RADIANS 3.1415926f/180.0f
// Octopus settings. Sizes are in body-local units, before the 0.5 glScaled in display().
#define MOUTH_RING_RADIUS (BODY_RADIUS * 0.2)     // Centre of the mouth to the middle of its tube.
#define MOUTH_TUBE_RADIUS (BODY_RADIUS * 0.06)    // Thickness of the mouth's tube.
#define BUBBLE_COUNT 4
#define BUBBLE_CYCLES_PER_SEC 0.5f                // Each bubble takes 2 s to rise and fade.


/* Tentacle SHAPE */
#define TENT_COUNT 8
#define TENT_LENGTH 1.4          // Body units.
#define TENT_RADIUS 0.2          // Thickness at the root.
#define TENT_TIP_RADIUS 0.05     // Thickness at the tip.
#define TENT_SEGMENTS 5          // Fewer segments = more angular.
#define TENT_SIDES 8             // Low number = lowpoly look.
#define TENT_BEND 0.0            // Degrees between segments. 0 = rigid and straight.
#define TENT_TILT 60.0           // Degrees below horizontal (0 = flat out, 90 = straight down).
#define TENT_ROOT_ELEV 55.0      // How far below the body's equator the roots sit.

/* Tentacle SWAY (constant animation) */
#define SWAY_DEG 10.0f           // How far each tentacle swings.
#define SWAY_SPEED 3.0f          // Radians per second.
#define SWAY_OFFSET 0.8f         // Phase shift between neighbours (0 = all in sync).

/* Tentacle SPIN (driven by spinLevel) */
#define SPIN_DPS 120.0f          // Ring spin at full spinLevel, degrees per second.

// Seabed settings.
#define SEABED_SQUARES_PER_SIDE 75   // Number of squares along each side of the grid.
#define SEABED_SQUARE_SIZE 2.0f      // Side length of one square 
#define SEABED_Y (-5.0f)             
#define SEABED_TEX_SCALE 0.06f       // One copy of the image every ~16 m.
GLuint seabedDL;
GLuint seabedTex;

// Cant go below seabed.
#define OCTOPUS_HALF_HEIGHT 1.0f
#define MIN_OCTOPUS_Y (SEABED_Y + OCTOPUS_HALF_HEIGHT)   

// Camera settings (metres, since 1 GL unit = 1 m).
#define CAMERA_BEHIND 5.0   // Metres behind the octopus, along its heading.
#define CAMERA_ABOVE  1.0   // Metres above the octopus (5 behind + 2 up = 7 m away).
#define PROPELLER_DEG_PER_SEC 600.0f   // About 100 RPM.
#define TURN_RATE_DEG_PER_SEC 90.0   // Yaw speed.
#define SPIN_UP_TIME 0.6f             // Seconds from stopped to full speed.
#define SPIN_DOWN_TIME 0.8f           // Seconds to wind down after the keys are released.
#define MOVE_SPIN_THRESHOLD 0.5f      // Octopus can't translate until spinLevel reaches this.

/* Seaweed */
#define SEAWEED_COUNT 80
#define SEAWEED_SEGMENTS 8
#define SEAWEED_HEIGHT 15.0        
#define SEAWEED_RADIUS 0.1       
#define SEAWEED_BEND 1.0  
#define SEAWEED_SIDES 5            // Pentagon stalk = chunky low-poly look.
#define SEAWEED_LEAF_SCALE 1.0f
#define SEAWEED_SWAY_DEG 2.0f
#define SEAWEED_FIELD_HALF 40.0f  
#define SEAWEED_SWAY_SPEED 1.0f   

/* Crashed lifepod */
#define POD_RADIUS 3.0f                               // Corner-to-centre of the octagon.
#define POD_HEIGHT 4.5f
#define POSTER_W 2.0f
#define POSTER_H 3.0f

#define POD_RADIUS 3.0f
#define POD_GROUND (SEABED_Y)                         // The pad is flat at SEABED_Y.
#define POD_Y (POD_GROUND + POD_RADIUS * 0.5f)        // Centre; the lowest 40% is buried.

#define FLOAT_TUBE 0.7f                               // Thickness of the orange ring.
#define FLOAT_RING_R 3.4f                             // Centre of ring to centre of tube.

GLfloat POD_AMBIENT[4] = { 0.20f, 0.09f, 0.04f, 1.0f };
GLfloat POD_DIFFUSE[4] = { 0.45f, 0.57f, 0.62f, 1.0f };
GLfloat POD_SPECULAR[4] = { 0.55f, 0.55f, 0.55f, 1.0f };
GLfloat LENS_COLOUR[4] = { 0.90f, 0.85f, 0.60f, 1.0f };
GLfloat LENS_GLOW[4] = { 1.00f, 0.90f, 0.50f, 1.0f };
GLfloat POSTER_FRAME[4] = { 0.12f, 0.12f, 0.14f, 1.0f };

GLfloat FLOAT_AMBIENT[4] = { 0.30f, 0.12f, 0.00f, 1.0f };
GLfloat FLOAT_DIFFUSE[4] = { 1.00f, 0.45f, 0.05f, 1.0f };   // Bright orange.
GLUquadricObj* podQuadric;

GLuint posterTex = 0;

// Flicker state for the two headlights.
int   podLightOn[2] = { 1, 1 };
float podLightTimer[2] = { 0.5f, 1.2f };
static const GLenum podLightId[2] = { GL_LIGHT1, GL_LIGHT2 };

GLUquadricObj* seaweedQuadric;
GLfloat VINE_STALK[4] = { 0.20f, 0.35f, 0.12f, 1.0f };
GLfloat VINE_LEAF[4] = { 0.15f, 0.55f, 0.18f, 1.0f };
GLfloat VINE_POD[4] = { 0.95f, 0.65f, 0.10f, 1.0f };

GLuint seabedDL;                 // Display list ID.
GLuint seaweedDL;                 // Display list ID.

float seaweedX[SEAWEED_COUNT];
float seaweedZ[SEAWEED_COUNT];
float seaweedScale[SEAWEED_COUNT];
float seaweedPhase[SEAWEED_COUNT];

GLfloat SEAWEED_GREEN[4] = { 0.10f, 0.55f, 0.20f, 1.0f };

float seabedHeight(float x, float z);

float bubble_phase = 0;
float sway_phase = 0;
float seaweed_phase = 0;
float spin_angle = 0;
float spinLevel = 0;                  // 0 = stopped, 1 = full speed. Starts switched off.
float objectLocation[3] = {-12, -3, 0 }; // x, y (up), z
float propeller_angle = 0;

const float moveSpeed = 3.0f; // metres per second
double heading = 0;                  // Degrees. 0 = facing +X, increasing = anticlockwise from above.

// Render objects as filled polygons (1) or wireframes (0). Default filled.
int renderFillEnabled = 1;

int bubblesEnabled = 1;
int smoothOn = 1;

// window dimensions
GLint windowWidth = 800;
GLint windowHeight = 600;

// Two sand tones, alternated in a checkerboard so the individual squares are visible.
// These have 4 components because glMaterialfv reads r, g, b and a.
GLfloat SAND_LIGHT[4] = { 0.62f, 0.56f, 0.38f, 1.0f };
GLfloat SAND_DARK[4] = { 0.62f, 0.56f, 0.38f, 1.0f };

GLUquadricObj* sphereQuadric;
GLUquadricObj* cylinderQuadric;

// a material that is all zeros
GLfloat zeroMaterial[] = { 0.0, 0.0, 0.0, 1.0 };
// a white diffuse material
GLfloat whiteDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
// a white specular material
GLfloat whiteSpecular[] = { 1, 1, 1, 1.0 };

// other colours
GLfloat PALE_GREEN[3] = { 0.596f, 0.984f, 0.596f };
GLfloat BEAK_BROWN[3] = { 0.545f, 0.27f, 0.0745f };
GLfloat YELLOW[3] = { 0.984f, 0.984f, 0.596f };
GLfloat BRIGHT_YELLOW[3] = { 1.0f, 1.0f, 0.0f };
GLfloat BLACK[3] = { 0.0f, 0.0f, 0.0f };
GLfloat WHITE[3] = { 1.0f, 1.0f, 1.0f };
GLfloat PLUM[3] = { 0.424f, 0.098f, 0.549f };
//rgb01(0.424, 0.098, 0.549)
GLfloat MOUTH_PINK[4] = { 0.98f, 0.60f, 0.75f, 1.0f };

GLfloat noShininess = 5.0;
GLfloat lowShininess = 25.0;
GLfloat medShininess = 50.0;
GLfloat highShininess = 100.0;


/******************************************************************************
 * Entry Point (don't put anything except the main function here)
 ******************************************************************************/

void main(int argc, char** argv)
{
	// Initialize the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Animation");

	// Set up the scene.
	init();

	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(specialKeyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutSpecialUpFunc(specialKeyReleased);
	glutIdleFunc(idle);

	// Record when we started rendering the very first frame (which should happen after we call glutMainLoop).
	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

	// Enter the main drawing loop (this will never return).
	glutMainLoop();
}

/******************************************************************************
 * GLUT Callbacks (don't add any other functions here)
 ******************************************************************************/

 /*
	 Called when GLUT wants us to (re)draw the current animation frame.

	 Note: This function must not do anything to update the state of our simulated
	 world. Animation (moving or rotating things, responding to keyboard input,
	 etc.) should only be performed within the think() function provided below.
 */
void display(void)
{
	/*
		TEMPLATE: REPLACE THIS COMMENT WITH YOUR DRAWING CODE

		Separate reusable pieces of drawing code into functions, which you can add
		to the "Animation-Specific Functions" section below.

		Remember to add prototypes for any new functions to the "Animation-Specific
		Function Prototypes" section near the top of this template.
	*/

	if (smoothOn)
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (!renderFillEnabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Camera position relative to the object, in metres!!
	double r = heading * (3.1415926 / 180.0);
	double at_x = objectLocation[0];
	double at_y = objectLocation[1];
	double at_z = objectLocation[2];

	// Target distance from the bird is about 5 m (e.g. 3, 3, 3), 
	double eyeX = at_x - CAMERA_BEHIND * cos(r);
	double eyeY = at_y + CAMERA_ABOVE;
	double eyeZ = at_z + CAMERA_BEHIND * sin(r);

	gluLookAt(eyeX, eyeY, eyeZ,
		at_x, at_y, at_z,
		0.0, 1.0, 0.0);

	updateFog(eyeX, eyeY, eyeZ);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // Moved here so it uses the new colour.

	setupPodLights();      // after gluLookAt

	//drawOriginMarker();
	drawSeabed();
	drawSeaweed();
	drawPod();
	drawPoster();
	drawPodBeams();
	glTranslated(objectLocation[0], objectLocation[1], objectLocation[2]);
	glRotated(heading, 0, 1, 0);
	glScaled(0.5, 0.5, 0.5); // 1 m body (BODY_RADIUS 1.0 -> 0.5 m radius)
	drawOctopus();
	drawTentacleRing();

	glutSwapBuffers();
}

/*
	Called when the OpenGL window has been resized.
*/
void reshape(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, windowWidth, windowHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Perspective projection in metres: 60 degree vertical FOV. 
	// Target near plane is 0.1 m (10 cm) and far plane 500 m for a 1 m bird.
	gluPerspective(60, (float)windowWidth / (float)windowHeight, 1.0, 5000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is pressed.
*/
void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is pressed, we do two things:
			(1) Update motionKeyStates to record that the key is held down. We use
				this later in the keyReleased callback.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. The most recent key always "wins" (e.g. if
				you're holding down KEY_MOVE_LEFT then also pressed KEY_MOVE_RIGHT,
				our object will immediately start moving right).
		*/

	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_FORWARD;
		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_BACKWARD;
		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_LEFT;
		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_RIGHT;
		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Rather than using literals (e.g. "t" for spotlight), create a new KEY_
			definition in the "Keyboard Input Handling Setup" section of this file.
			For example, refer to the existing keys used here (KEY_MOVE_FORWARD,
			KEY_MOVE_LEFT, KEY_EXIT, etc).
		*/

	case KEY_RENDER_FILL:
		renderFillEnabled = !renderFillEnabled;
		break;
	case KEY_TOGGLE_BUBBLES:
		bubblesEnabled = !bubblesEnabled;
		break;
	case KEY_EXIT:
		exit(0);
		break;
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is pressed.
*/
void specialKeyPressed(int key, int x, int y)
{
	switch (key) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyPressed.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_UP;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_DOWN;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_DOWN;
		keyboardMotion.Yaw = MOTION_ANTICLOCKWISE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_DOWN;
		keyboardMotion.Yaw = MOTION_CLOCKWISE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			Rather than directly using the GLUT constants (e.g. GLUT_KEY_F1), create
			a new SP_KEY_ definition in the "Keyboard Input Handling Setup" section of
			this file. For example, refer to the existing keys used here (SP_KEY_MOVE_UP,
			SP_KEY_TURN_LEFT, etc).
		*/
	}
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is released.
*/
void keyReleased(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is released, we do two things:
			(1) Update motionKeyStates to record that the key is no longer held down;
				we need to know when we get to step (2) below.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. This gets a little complicated to ensure
				the controls work smoothly. When the user releases a key that moves
				in one direction (e.g. KEY_MOVE_RIGHT), we check if its "opposite"
				key (e.g. KEY_MOVE_LEFT) is pressed down. If it is, we begin moving
				in that direction instead. Otherwise, we just stop moving.
		*/
	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveBackward == KEYSTATE_DOWN) ? MOTION_BACKWARD : MOTION_NONE;
		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveForward == KEYSTATE_DOWN) ? MOTION_FORWARD : MOTION_NONE;
		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveRight == KEYSTATE_DOWN) ? MOTION_RIGHT : MOTION_NONE;
		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveLeft == KEYSTATE_DOWN) ? MOTION_LEFT : MOTION_NONE;
		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Note: If you only care when your key is first pressed down, you don't have to
			add anything here. You only need to put something in keyReleased if you care
			what happens when the user lets go, like we do with our movement keys above.
			For example: if you wanted a spotlight to come on while you held down "t", you
			would need to set a flag to turn the spotlight on in keyPressed, and update the
			flag to turn it off in keyReleased.
		*/
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is released.
*/
void specialKeyReleased(int key, int x, int y)
{
	switch (key) {
		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyReleased.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveDown == KEYSTATE_DOWN) ? MOTION_DOWN : MOTION_NONE;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_UP;
		keyboardMotion.Heave = (motionKeyStates.MoveUp == KEYSTATE_DOWN) ? MOTION_UP : MOTION_NONE;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnRight == KEYSTATE_DOWN) ? MOTION_CLOCKWISE : MOTION_NONE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnLeft == KEYSTATE_DOWN) ? MOTION_ANTICLOCKWISE : MOTION_NONE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			As per keyReleased, you only need to handle the key here if you want something
			to happen when the user lets go. If you just want something to happen when the
			key is first pressed, add you code to specialKeyPressed instead.
		*/
	}
}

/*
	Called by GLUT when it's not rendering a frame.

	Note: We use this to handle animation and timing. You shouldn't need to modify
	this callback at all. Instead, place your animation logic (e.g. moving or rotating
	things) within the think() method provided with this template.
*/
void idle(void)
{
	// Wait until it's time to render the next frame.

	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) - frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		// This frame took less time to render than the ideal FRAME_TIME: we'll suspend this thread for the remaining time,
		// so we're not taking up the CPU until we need to render another frame.
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}

	// Begin processing the next frame.

	frameStartTime = glutGet(GLUT_ELAPSED_TIME); // Record when we started work on the new frame.

	think(); // Update our simulated world before the next call to display().

	glutPostRedisplay(); // Tell OpenGL there's a new frame ready to be drawn.
}

/******************************************************************************
 * Animation-Specific Functions (Add your own functions at the end of this section)
 ******************************************************************************/

 /*
	 Initialise OpenGL and set up our scene before we begin the render loop.
 */
void init(void)
{
	// define the light color and intensity
	GLfloat ambientLight[] = { 0.0, 0.0, 0.0, 1.0 };  // relying on global ambient
	GLfloat diffuseLight[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };

	//  the global ambient light level
	GLfloat globalAmbientLight[] = { 0.4f, 0.4f, 0.4f, 1.0f };

	// define the color and intensity for light 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, diffuseLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, specularLight);

	// enable lighting 
	glEnable(GL_LIGHTING);
	// enable light 0

	// Deep sea blue for the water behind and beyond the seabed.
	glClearColor(0.02f, 0.16f, 0.30f, 1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);

	sphereQuadric = gluNewQuadric();
	cylinderQuadric = gluNewQuadric();
	seaweedQuadric = gluNewQuadric();
	gluQuadricNormals(seaweedQuadric, GLU_FLAT);
	gluQuadricDrawStyle(seaweedQuadric, GLU_FILL);
	podQuadric = gluNewQuadric();
	gluQuadricNormals(podQuadric, GLU_FLAT);   // Flat facets keep the octagon crisp.

	initLights();

	// Anything that relies on lighting or specifies normals must be initialised after initLights.
	createSeabedTexture();
	createSeabedDisplayList();
	createSeaweedDisplayList();
	initSeaweed();

	createPosterTexture();
	initPodLights();
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP);   
}

/*
	Advance our animation by FRAME_TIME milliseconds.

	Note: Our template's GLUT idle() callback calls this once before each new
	frame is drawn, EXCEPT the very first frame drawn after our application
	starts. Any setup required before the first frame is drawn should be placed
	in init().
*/
void think(void)
{
	/*
		TEMPLATE: REPLACE THIS COMMENT WITH YOUR ANIMATION/SIMULATION CODE

		In this function, we update all the variables that control the animated
		parts of our simulated world. For example: if you have a moving box, this is
		where you update its coordinates to make it move. If you have something that
		spins around, here's where you update its angle.

		NOTHING CAN BE DRAWN IN HERE: you can only update the variables that control
		how everything will be drawn later in display().

		How much do we move or rotate things? Because we use a fixed frame rate, we
		assume there's always FRAME_TIME milliseconds between drawing each frame. So,
		every time think() is called, we need to work out how far things should have
		moved, rotated, or otherwise changed in that period of time.

		Movement example:
		* Let's assume a distance of 1.0 GL units is 1 metre.
		* Let's assume we want something to move 2 metres per second on the x axis
		* Each frame, we'd need to update its position like this:
			x += 2 * (FRAME_TIME / 1000.0f)
		* Note that we have to convert FRAME_TIME to seconds. We can skip this by
		  using a constant defined earlier in this template:
			x += 2 * FRAME_TIME_SEC;

		Rotation example:
		* Let's assume we want something to do one complete 360-degree rotation every
		  second (i.e. 60 Revolutions Per Minute, or RPM).
		* Each frame, we'd need to update our object's angle like this (we'll use the
		  FRAME_TIME_SEC constant as per the example above):
			a += 360 * FRAME_TIME_SEC;

		This works for any type of "per second" change: just multiply the amount you'd
		want to move in a full second by FRAME_TIME_SEC, and add or subtract that
		from whatever variable you're updating.

		You can use this same approach to animate other things like color, opacity,
		brightness of lights, etc.
	*/

	/*
		Keyboard motion handler: complete this section to make your "player-controlled"
		object respond to keyboard input.
	*/

	double rad = heading * (3.1415926 / 180.0);
	float movedThisFrame = moveSpeed * FRAME_TIME_SEC;   // Metres moved this frame.

	// Does the player want to translate in any direction? (Yaw doesn't count.)
	int wantsToMove = (keyboardMotion.Surge != MOTION_NONE ||
		keyboardMotion.Sway != MOTION_NONE ||
		keyboardMotion.Heave != MOTION_NONE);

	// Spin up while a movement key is held, wind down otherwise.
	if (wantsToMove)
		spinLevel += FRAME_TIME_SEC / SPIN_UP_TIME;
	else
		spinLevel -= FRAME_TIME_SEC / SPIN_DOWN_TIME;
	if (spinLevel > 1.0f) spinLevel = 1.0f;
	if (spinLevel < 0.0f) spinLevel = 0.0f;

	propeller_angle = fmodf(propeller_angle + PROPELLER_DEG_PER_SEC * spinLevel * FRAME_TIME_SEC, 360.0f);
	spin_angle = fmodf(spin_angle - SPIN_DPS * spinLevel * FRAME_TIME_SEC, 360.0f);

	// Yaw does not need the propeller or legs.
	if (keyboardMotion.Yaw != MOTION_NONE) {
		/* TEMPLATE: Turn your object right (clockwise) if .Yaw < 0, or left (anticlockwise) if .Yaw > 0 */
		heading += keyboardMotion.Yaw * TURN_RATE_DEG_PER_SEC * FRAME_TIME_SEC;
		if (heading >= 360.0) heading -= 360.0;
		if (heading < 0.0)    heading += 360.0;
	}

	// Translation only happens once the propeller and legs are spinning fast enough.
	if (spinLevel >= MOVE_SPIN_THRESHOLD) {
		if (keyboardMotion.Surge != MOTION_NONE) {
			/* TEMPLATE: Move your object backward if .Surge < 0, or forward if .Surge > 0 */
			objectLocation[0] += keyboardMotion.Surge * (float)cos(rad) * movedThisFrame;
			objectLocation[2] -= keyboardMotion.Surge * (float)sin(rad) * movedThisFrame;
		}
		if (keyboardMotion.Sway != MOTION_NONE) {
			/* TEMPLATE: Move (strafe) your object left if .Sway < 0, or right if .Sway > 0 */
			objectLocation[0] += keyboardMotion.Sway * (float)sin(rad) * movedThisFrame;
			objectLocation[2] += keyboardMotion.Sway * (float)cos(rad) * movedThisFrame;
		}
		if (keyboardMotion.Heave != MOTION_NONE) {
			/* TEMPLATE: Move your object down if .Heave < 0, or up if .Heave > 0 */
			objectLocation[1] += keyboardMotion.Heave * movedThisFrame;
		}
	}

	{
		float floorY = seabedHeight(objectLocation[0], objectLocation[2]) + OCTOPUS_HALF_HEIGHT;
		if (objectLocation[1] < floorY)
			objectLocation[1] = floorY;
	}

	// Octopus can't swim into the pod: push it back out to the surface of the sphere.
	{
		float dx = objectLocation[0];
		float dy = objectLocation[1] - POD_Y;
		float dz = objectLocation[2];
		float dist = sqrtf(dx * dx + dy * dy + dz * dz);
		float minDist = POD_RADIUS + OCTOPUS_COLLIDE_RADIUS;

		if (dist < minDist)
		{
			if (dist < 0.001f) { dx = -1.0f; dy = 0.0f; dz = 0.0f; dist = 1.0f; }   // Exactly at the centre.
			objectLocation[0] = dx / dist * minDist;
			objectLocation[1] = POD_Y + dy / dist * minDist;
			objectLocation[2] = dz / dist * minDist;
		}
	}

	bubble_phase = fmodf(bubble_phase + BUBBLE_CYCLES_PER_SEC * FRAME_TIME_SEC, 1.0f);
	sway_phase = fmodf(sway_phase + SWAY_SPEED * FRAME_TIME_SEC, 2.0f * 3.1415926f);
	seaweed_phase = fmodf(seaweed_phase + SEAWEED_SWAY_SPEED * FRAME_TIME_SEC, 2.0f * 3.1415926f);
	updatePodLights();
}

/*
	Initialise OpenGL lighting before we begin the render loop.

	Note (advanced): If you're using dynamic lighting (e.g. lights that move around, turn on or
	off, or change colour) you may want to replace this with a drawLights function that gets called
	at the beginning of display() instead of init().
*/

void initLights(void)
{
	// Simple lighting setup
	GLfloat globalAmbient[] = { 0.25, 0.25f, 0.25f, 1 };
	GLfloat lightPosition[] = { 5.0f, 5.0f, 5.0f, 1.0f };
	GLfloat ambientLight[] = { 0, 0, 0, 1 };
	GLfloat diffuseLight[] = { 1, 1, 1, 1 };
	GLfloat specularLight[] = { 1, 1, 1, 1 };

	// Configure global ambient lighting.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	// Configure Light 0.
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Make GL normalize the normal vectors we supply.
	glEnable(GL_NORMALIZE);

}

static float mixf(float a, float b, float t) { return a + (b - a) * t; }

// Sets fog colour/density (and the background to match) from the camera position.
//  - Higher up (nearer the surface): lighter, clearer, teal water.
//  - Lower down (near the seabed):   darker, denser, deep blue.
//  - Near the lifepod:               slightly murkier and greener, like stirred-up silt.
void updateFog(double eyeX, double eyeY, double eyeZ)
{
	const float shallow[3] = { 0.12f, 0.52f, 0.45f };   
	const float deep[3] = { 0.02f, 0.14f, 0.12f };   
	const float murk[3] = { 0.14f, 0.30f, 0.14f };   
	const float shallowDensity = 0.020f, deepDensity = 0.060f;

	float col[4], density, podDist, depthT, podT;
	int k;

	// 0 at the surface (y = 0), 1 at 5 m down.
	depthT = (float)(-eyeY) / 5.0f;
	if (depthT < 0.0f) depthT = 0.0f;
	if (depthT > 1.0f) depthT = 1.0f;

	// 1 right next to the pod, fading to 0 by 25 m away (smoothstep).
	podDist = sqrtf((float)(eyeX * eyeX + eyeZ * eyeZ));
	podT = 1.0f - podDist / 25.0f;
	if (podT < 0.0f) podT = 0.0f;
	podT = podT * podT * (3.0f - 2.0f * podT);

	for (k = 0; k < 3; k++)
	{
		col[k] = mixf(shallow[k], deep[k], depthT);
		col[k] = mixf(col[k], murk[k], 0.5f * podT);
	}
	col[3] = 1.0f;
	density = mixf(shallowDensity, deepDensity, depthT) * (1.0f + 0.5f * podT);

	glFogfv(GL_FOG_COLOR, col);
	glFogf(GL_FOG_DENSITY, density);
	glClearColor(col[0], col[1], col[2], 1.0f);   // Background matches the fog.
}

// Reads the next whole number from a PPM header/body, skipping spaces and # comments.
static int ppmReadInt(FILE* fp)
{
	int c = fgetc(fp), v = 0;
	while (c != EOF && (c == '#' || isspace(c)))
	{
		if (c == '#') { while (c != '\n' && c != EOF) c = fgetc(fp); }
		else c = fgetc(fp);
	}
	if (c == EOF) return -1;
	while (c >= '0' && c <= '9') { v = v * 10 + (c - '0'); c = fgetc(fp); }
	return v;   // Also consumes the one whitespace after the number, as P6 requires.
}

GLubyte* loadPPM(const char* filename, int* w, int* h)
{
	FILE* fp = fopen(filename, "rb");
	GLubyte* pixels = NULL;
	int c0, c1, type, maxVal, i, n;

	if (!fp) { printf("Could not open %s\n", filename); return NULL; }

	c0 = fgetc(fp);
	c1 = fgetc(fp);
	printf("First two bytes of file: '%c' '%c' (codes %d %d)\n", c0, c1, c0, c1);
	if (c0 != 'P') { printf("Not a PPM: file doesn't start with P\n"); fclose(fp); return NULL; }
	type = c1 - '0';
	if (type != 3 && type != 6) { printf("Not P3/P6, got P%d\n", type); fclose(fp); return NULL; }

	*w = ppmReadInt(fp);
	*h = ppmReadInt(fp);
	maxVal = ppmReadInt(fp);
	printf("Header: P%d, width %d, height %d, max %d\n", type, *w, *h, maxVal);
	if (*w <= 0 || *h <= 0) { printf("Bad width/height\n"); fclose(fp); return NULL; }
	if (maxVal <= 0 || maxVal > 255) { printf("Unsupported max value (16-bit PPM?)\n"); fclose(fp); return NULL; }

	n = (*w) * (*h) * 3;
	pixels = (GLubyte*)malloc(n);
	if (!pixels) { printf("Out of memory\n"); fclose(fp); return NULL; }

	if (type == 6)
	{
		int got = (int)fread(pixels, 1, n, fp);
		if (got != n) { printf("Pixel data too short: got %d of %d bytes\n", got, n); free(pixels); pixels = NULL; }
	}
	else
	{
		for (i = 0; i < n; i++)
			pixels[i] = (GLubyte)(ppmReadInt(fp) * 255 / maxVal);
	}
	fclose(fp);
	return pixels;
}

void createSeabedTexture(void)
{
	int w, h;
	GLubyte* pixels = loadPPM("seabed.ppm", &w, &h);
	if (!pixels) { printf("Seabed texture FAILED to load\n"); return; }
	printf("Seabed texture loaded: %d x %d\n", w, h);

	glGenTextures(1, &seabedTex);
	glBindTexture(GL_TEXTURE_2D, seabedTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
}

float seabedHeight(float x, float z)
{
	float h = SEABED_Y
		+ 0.80f * sinf(0.10f * x + 0.3f * sinf(0.05f * z))
		+ 0.25f * sinf(0.06f * x + 0.08f * z);

	// Blend to a flat pad within ~6 m of the pod, back to dunes by ~14 m.
	float d = sqrtf(x * x + z * z);
	float t = (d - 6.0f) / 8.0f;
	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;
	t = t * t * (3.0f - 2.0f * t);           // Smoothstep: no visible edge.
	return SEABED_Y + (h - SEABED_Y) * t;
}

// One vertex: normal from the slope, plus texture coordinates from position.
static void seabedVertex(float x, float z)
{
	float e = 0.1f;
	float dx = (seabedHeight(x + e, z) - seabedHeight(x - e, z)) / (2.0f * e);
	float dz = (seabedHeight(x, z + e) - seabedHeight(x, z - e)) / (2.0f * e);
	glNormal3f(-dx, 1.0f, -dz);
	glTexCoord2f(x * SEABED_TEX_SCALE, z * SEABED_TEX_SCALE);
	glVertex3f(x, seabedHeight(x, z), z);
}

void createSeabedDisplayList(void)
{
	int i, j;
	float half = SEABED_SQUARES_PER_SIDE * SEABED_SQUARE_SIZE / 2.0f;
	GLfloat white[4] = { 0.8f, 0.85f, 0.8f, 1 };   // Slightly darker and a bit less blue.

	seabedDL = glGenLists(1);
	glNewList(seabedDL, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);   // Don't tint the texture.
	glBegin(GL_TRIANGLES);
	for (i = 0; i < SEABED_SQUARES_PER_SIDE; i++)
	{
		for (j = 0; j < SEABED_SQUARES_PER_SIDE; j++)
		{
			float x0 = -half + i * SEABED_SQUARE_SIZE;
			float z0 = -half + j * SEABED_SQUARE_SIZE;
			float x1 = x0 + SEABED_SQUARE_SIZE;
			float z1 = z0 + SEABED_SQUARE_SIZE;

			seabedVertex(x0, z0); seabedVertex(x0, z1); seabedVertex(x1, z1);
			seabedVertex(x0, z0); seabedVertex(x1, z1); seabedVertex(x1, z0);
		}
	}
	glEnd();
	glEndList();
}

void drawSeabed(void)
{
	glPolygonMode(GL_FRONT_AND_BACK, renderFillEnabled ? GL_FILL : GL_LINE);
	glDisable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, seabedTex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);   // Keeps the lighting.
	glCallList(seabedDL);
	glDisable(GL_TEXTURE_2D);   // Keeps the octopus and seaweed untextured.
}

// Reads an uncompressed 24-bit BMP. Returns malloc'd RGB pixels, or NULL on failure.
GLubyte* loadBMP(const char* filename, int* w, int* h)
{
	FILE* fp = fopen(filename, "rb");
	unsigned char header[54];
	int dataOffset, bpp, rowSize, x, y;
	GLubyte* pixels;
	unsigned char* row;

	if (!fp) { printf("Could not open %s\n", filename); return NULL; }
	if (fread(header, 1, 54, fp) != 54 || header[0] != 'B' || header[1] != 'M')
	{
		printf("%s is not a real BMP\n", filename); fclose(fp); return NULL;
	}

	dataOffset = *(int*)&header[10];
	*w = *(int*)&header[18];
	*h = *(int*)&header[22];
	bpp = *(short*)&header[28];
	if (bpp != 24 || *h <= 0) { printf("BMP must be 24-bit\n"); fclose(fp); return NULL; }

	rowSize = ((*w * 3 + 3) / 4) * 4;          // Rows are padded to a multiple of 4 bytes.
	pixels = (GLubyte*)malloc((size_t)(*w) * (*h) * 3);
	row = (unsigned char*)malloc(rowSize);
	fseek(fp, dataOffset, SEEK_SET);

	for (y = 0; y < *h; y++)                   // BMP stores the bottom row first, same as OpenGL.
	{
		fread(row, 1, rowSize, fp);
		for (x = 0; x < *w; x++)
		{
			pixels[(y * (*w) + x) * 3 + 0] = row[x * 3 + 2];   // BGR to RGB.
			pixels[(y * (*w) + x) * 3 + 1] = row[x * 3 + 1];
			pixels[(y * (*w) + x) * 3 + 2] = row[x * 3 + 0];
		}
	}
	free(row);
	fclose(fp);
	return pixels;
}

void createPosterTexture(void)
{
	int w, h;
	GLubyte* pixels = loadBMP("poster.bmp", &w, &h);
	if (!pixels) { printf("Poster texture FAILED to load\n"); return; }

	glGenTextures(1, &posterTex);
	glBindTexture(GL_TEXTURE_2D, posterTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);   // One poster, no tiling.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
}

// Direction from the pod's centre to headlight i (0 = one side, 1 = the other).
// Both face -X, the side the octopus starts on, 30 degrees either side and slightly up.
static void podLampDir(int i, float d[3])
{
	float el = 15.0f * 3.1415926f / 180.0f;
	float az = 30.0f * 3.1415926f / 180.0f;
	float side = (i == 0) ? 1.0f : -1.0f;
	d[0] = -cosf(el) * cosf(az);
	d[1] = sinf(el);
	d[2] = side * cosf(el) * sinf(az);
}

// Constant light settings. Position and direction are set every frame in setupPodLights().
void initPodLights(void)
{
	GLfloat zero[4] = { 0, 0, 0, 1 };
	GLfloat beam[4] = { 1.0f, 0.95f, 0.70f, 1.0f };
	int i;
	for (i = 0; i < 2; i++)
	{
		glLightfv(podLightId[i], GL_AMBIENT, zero);
		glLightfv(podLightId[i], GL_DIFFUSE, beam);     // Lights 1 and 2 default to black.
		glLightfv(podLightId[i], GL_SPECULAR, beam);
		glLightf(podLightId[i], GL_SPOT_CUTOFF, 10.0f); // 30 degrees wide, from the centre to the edge of the cone.
		glLightf(podLightId[i], GL_SPOT_EXPONENT, 20.0f);
		glLightf(podLightId[i], GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf(podLightId[i], GL_LINEAR_ATTENUATION, 0.02f);
		glLightf(podLightId[i], GL_QUADRATIC_ATTENUATION, 0.005f);
	}
}

// Randomly toggles each lamp: long on, short off bursts. The second lamp is more broken.
void updatePodLights(void)
{
	int i;
	for (i = 0; i < 2; i++)
	{
		podLightTimer[i] -= FRAME_TIME_SEC;
		if (podLightTimer[i] <= 0.0f)
		{
			float r = (float)rand() / RAND_MAX;
			if (podLightOn[i]) { podLightOn[i] = 0; podLightTimer[i] = 0.03f + 0.15f * r; }
			else { podLightOn[i] = 1; podLightTimer[i] = (i == 0 ? 1.5f : 0.6f) * (0.2f + 1.5f * r); }
		}
	}
}

// Must be called in display() AFTER gluLookAt, because light positions go through the modelview matrix.
void setupPodLights(void)
{
	int i;
	float d[3];
	glPushMatrix();
	glTranslatef(0.0f, POD_Y, 0.0f);
	for (i = 0; i < 2; i++)
	{
		GLfloat pos[4], dir[3];
		podLampDir(i, d);
		pos[0] = d[0] * (POD_RADIUS + 0.3f);
		pos[1] = d[1] * (POD_RADIUS + 0.3f);
		pos[2] = d[2] * (POD_RADIUS + 0.3f);
		pos[3] = 1.0f;                                   // 1 = positional light.
		dir[0] = d[0]; dir[1] = d[1] - 0.35f; dir[2] = d[2];   // Tilted down toward the sand.
		glLightfv(podLightId[i], GL_POSITION, pos);
		glLightfv(podLightId[i], GL_SPOT_DIRECTION, dir);
		if (podLightOn[i]) glEnable(podLightId[i]); else glDisable(podLightId[i]);
	}
	glPopMatrix();
}

void drawPod(void)
{
	int i;
	float d[3];
	GLfloat noGlow[4] = { 0, 0, 0, 1 };

	gluQuadricDrawStyle(sphereQuadric, renderFillEnabled ? GLU_FILL : GLU_LINE);

	// Spherical hull
	glPushMatrix();
	glTranslatef(0.0f, POD_Y, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, POD_AMBIENT);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, POD_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, POD_SPECULAR);
	glMaterialf(GL_FRONT, GL_SHININESS, 60.0f);
	gluSphere(sphereQuadric, POD_RADIUS, 40, 40);

	// Headlight lenses on the surface
	for (i = 0; i < 2; i++)
	{
		podLampDir(i, d);
		glPushMatrix();
		glTranslatef(d[0] * POD_RADIUS, d[1] * POD_RADIUS, d[2] * POD_RADIUS);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, LENS_COLOUR);
		glMaterialfv(GL_FRONT, GL_EMISSION, podLightOn[i] ? LENS_GLOW : noGlow);
		gluSphere(sphereQuadric, 0.4, 10, 10);
		glMaterialfv(GL_FRONT, GL_EMISSION, noGlow);
		glPopMatrix();
	}
	glPopMatrix();

	// Orange flotation ring, around the sphere's lower half
	glPushMatrix();
	glTranslatef(0.0f, POD_GROUND + FLOAT_TUBE * 0.4f, 0.0f);
	glRotated(90, 1, 0, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, FLOAT_AMBIENT);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, FLOAT_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	renderFillEnabled ? glutSolidTorus(FLOAT_TUBE, FLOAT_RING_R, 12, 16)
		: glutWireTorus(FLOAT_TUBE, FLOAT_RING_R, 12, 16);
	glPopMatrix();
}

void drawPoster(void)
{
	float px = -4.0f, pz = 4.0f;                         // Next to the pod, facing the start side.
	float hw = POSTER_W / 2.0f - 0.08f, hh = POSTER_H / 2.0f - 0.08f;
	GLfloat white[4] = { 1, 1, 1, 1 };

	glPushMatrix();
	glTranslatef(px, seabedHeight(px, pz) + POSTER_H / 2.0f - 0.2f, pz);
	glRotated(-90, 0, 1, 0);                             // Front face points toward -X.

	// Thin backing board
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, POSTER_FRAME);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
	glPushMatrix();
	glScalef(POSTER_W, POSTER_H, 0.08f);
	glutSolidCube(1.0);
	glPopMatrix();

	// Textured picture, slightly in front of the board
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, posterTex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-hw, -hh, 0.045f);
	glTexCoord2f(1, 0); glVertex3f(hw, -hh, 0.045f);
	glTexCoord2f(1, 1); glVertex3f(hw, hh, 0.045f);
	glTexCoord2f(0, 1); glVertex3f(-hw, hh, 0.045f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void drawPodBeams(void)
{
	const int   SIDES = 20;
	const float LEN = 10.0f;                       // Beam length in metres.
	const float START_R = 0.2f;                    // Radius at the lamp.
	const float END_R = 3.0f;                      // Radius at the far end.
	const float ALPHA = 0.30f;                     // Brightness at the lamp (fades to 0).
	int i, s;
	float d[3];

	glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
	glDisable(GL_LIGHTING);                        // Beam is a flat colour, not lit.
	glDisable(GL_FOG);     // Restored by glPopAttrib at the end.
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);             // Additive: stacks like real light.
	glDepthMask(GL_FALSE);                         // Don't hide things behind the beam.

	for (i = 0; i < 2; i++)
	{
		float dx, dy, dz, len, angle;
		if (!podLightOn[i]) continue;

		podLampDir(i, d);
		dx = d[0]; dy = d[1] - 0.35f; dz = d[2];   // Same tilt as setupPodLights().
		len = sqrtf(dx * dx + dy * dy + dz * dz);
		dx /= len; dy /= len; dz /= len;
		angle = acosf(dz) * 180.0f / 3.1415926f;   // Angle from +Z to the beam direction.

		glPushMatrix();
		glTranslatef(d[0] * POD_RADIUS, POD_Y + d[1] * POD_RADIUS, d[2] * POD_RADIUS);
		glRotatef(angle, -dy, dx, 0.0f);           // Turn the cone's +Z axis onto the beam.

		glBegin(GL_TRIANGLE_STRIP);
		for (s = 0; s <= SIDES; s++)
		{
			float a = 2.0f * 3.1415926f * s / SIDES;
			glColor4f(1.0f, 0.95f, 0.70f, ALPHA);
			glVertex3f(START_R * cosf(a), START_R * sinf(a), 0.0f);
			glColor4f(1.0f, 0.95f, 0.70f, 0.0f);   // Fades to nothing at the far end.
			glVertex3f(END_R * cosf(a), END_R * sinf(a), LEN);
		}
		glEnd();
		glPopMatrix();
	}
	glPopAttrib();                                 // Restores lighting, blending, depth mask.
}

void drawOctopus(void)
{
	int i;
	double mouthZ;
	float t;
	GLfloat bubbleColour[4];

	renderFillEnabled ? gluQuadricDrawStyle(sphereQuadric, GLU_FILL) : gluQuadricDrawStyle(sphereQuadric, GLU_LINE);

	// body
	glPushMatrix();
	glRotated(90, 0, 1, 0); // rotate x and z axes 90 degree
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, PLUM);
	glMaterialfv(GL_FRONT, GL_SPECULAR, PLUM);
	glMaterialf(GL_FRONT, GL_SHININESS, medShininess);

	gluSphere(sphereQuadric, BODY_RADIUS, 50, 50);

	// left eye
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, BLACK);
	glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
	glMaterialf(GL_FRONT, GL_SHININESS, lowShininess);
	glTranslated(BODY_RADIUS * 0.3f, BODY_RADIUS * 0.6f, BODY_RADIUS * 0.7f);
	gluSphere(sphereQuadric, EYE_RADIUS, 50, 50);
	glPopMatrix();

	// right eye
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, BLACK);
	glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
	glMaterialf(GL_FRONT, GL_SHININESS, lowShininess);
	glTranslated(-BODY_RADIUS * 0.3f, BODY_RADIUS * 0.6f, BODY_RADIUS * 0.7f);
	gluSphere(sphereQuadric, EYE_RADIUS, 50, 50);
	glPopMatrix();

	// mouth: a torus lying flat against the front of the body (its hole points along z).
	// It is moved back from the beak's old position so the ring sits on the body's surface.
	mouthZ = sqrt(BODY_RADIUS * BODY_RADIUS - MOUTH_RING_RADIUS * MOUTH_RING_RADIUS);
	glPushMatrix();
	glTranslated(0.0, 0.0, mouthZ);
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MOUTH_PINK);
	glMaterialfv(GL_FRONT, GL_SPECULAR, MOUTH_PINK);
	glMaterialf(GL_FRONT, GL_SHININESS, lowShininess);
	// GLUT's torus takes the tube radius first, then the radius of the ring itself.
	renderFillEnabled ? glutSolidTorus(MOUTH_TUBE_RADIUS, MOUTH_RING_RADIUS, 20, 30)
		: glutWireTorus(MOUTH_TUBE_RADIUS, MOUTH_RING_RADIUS, 20, 30);
	glPopMatrix();

	// propeller
	glPushMatrix();
	//glTranslated(0.0, 0.0, BODY_RADIUS - BODY_RADIUS * 2.0); // move the propeller to the front of the bird
	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, PLUM);
	glMaterialfv(GL_FRONT, GL_SPECULAR, PLUM);
	glMaterialf(GL_FRONT, GL_SHININESS, lowShininess);
	glNormal3f(0, 1, 0); // normal for the propeller blades (pointing up the y axis)

	glTranslated(0, 0, -BODY_RADIUS); // move the propeller to the back of the bird
	glRotated(90, 1, 0, 0);
	glRotated(propeller_angle, 0, 1, 0); // rotate the propeller around the y axis

	glBegin(GL_TRIANGLES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.5f, 0, 0.2f);
	glVertex3f(0.5f, 0, -0.2f);
	glEnd();

	glNormal3f(0, 1, 0);
	glBegin(GL_TRIANGLES);
	glVertex3f(0, 0, 0);
	glVertex3f(-0.5f, 0, 0.2f);
	glVertex3f(-0.5f, 0, -0.2f);
	glEnd();
	glPopMatrix();

	// bubbles: translucent spheres that rise from the mouth, grow and fade.
	// Drawn last so they blend over everything else.

	// bubbles ...
	if (bubblesEnabled)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE); // see-through bubbles shouldn't hide what is behind them
		glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
		glMaterialf(GL_FRONT, GL_SHININESS, highShininess);
		for (i = 0; i < BUBBLE_COUNT; i++)
		{
			t = fmodf(bubble_phase + (float)i / BUBBLE_COUNT, 1.0f); // 0 = just left the mouth, 1 = gone

			bubbleColour[0] = 0.7f;
			bubbleColour[1] = 0.9f;
			bubbleColour[2] = 1.0f;
			bubbleColour[3] = 0.7f * (1.0f - t); // fade out as it rises
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bubbleColour);

			glPushMatrix();
			glTranslated(BODY_RADIUS * 0.08 * sin(t * 10.0 + i),  // small sideways wobble
				BODY_RADIUS * (0.1 + 1.4 * t),                     // rise
				BODY_RADIUS * (1.2 + 0.3 * t));                    // drift forward, past the mouth
			gluSphere(sphereQuadric, BODY_RADIUS * (0.05 + 0.10 * t), 16, 16); // grow as it rises
			glPopMatrix();
		}
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	glPopMatrix(); //end outer push
}

// SHAPE only
void drawTentacle(void)
{
	int s;
	double segLen = TENT_LENGTH / TENT_SEGMENTS;
	double r0, r1;

	for (s = 0; s < TENT_SEGMENTS; s++)
	{
		// Radius shrinks evenly from root to tip.
		r0 = TENT_RADIUS + (TENT_TIP_RADIUS - TENT_RADIUS) * s / TENT_SEGMENTS;
		r1 = TENT_RADIUS + (TENT_TIP_RADIUS - TENT_RADIUS) * (s + 1) / TENT_SEGMENTS;

		gluCylinder(cylinderQuadric, r0, r1, segLen, TENT_SIDES, 1);
		glTranslated(0, 0, segLen);                   // Move to the end of this segment.
		gluSphere(sphereQuadric, r1, TENT_SIDES, TENT_SIDES);   // Joint.
		glRotated(TENT_BEND, 1, 0, 0);                // Zero = straight.
	}
}

// Places all tentacles, then applies SPIN (whole ring) and SWAY (each tentacle).
void drawTentacleRing(void)
{
	int i;
	double elev = TENT_ROOT_ELEV * 3.1415926 / 180.0;

	glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, PLUM);
	glMaterialfv(GL_FRONT, GL_SPECULAR, PLUM);
	glMaterialf(GL_FRONT, GL_SHININESS, medShininess);
	gluQuadricDrawStyle(cylinderQuadric, renderFillEnabled ? GLU_FILL : GLU_LINE);

	glPushMatrix();
	glRotated(spin_angle, 0, 1, 0);                        // SPIN: whole ring.
	for (i = 0; i < TENT_COUNT; i++)
	{
		glPushMatrix();
		glRotated(-i * 360.0 / TENT_COUNT, 0, 1, 0);       // Face this tentacle's direction.
		glTranslated(BODY_RADIUS * cos(elev) * 0.9,        // Root slightly inside the body.
			-BODY_RADIUS * sin(elev) * 0.9, 0.0);
		glRotated(90, 0, 1, 0);                            // Local +Z points outward.
		glRotated(TENT_TILT, 1, 0, 0);                     // SHAPE: tilt downward.
		glRotated(SWAY_DEG * sinf(sway_phase + i * SWAY_OFFSET), 1, 0, 0);   // SWAY.
		drawTentacle();
		glPopMatrix();
	}
	glPopMatrix();
}

// Scatter the strands once, so they stay put every frame.
void initSeaweed(void)
{
	int i;
	srand(42);   // Fixed seed = same layout every run.
	for (i = 0; i < SEAWEED_COUNT; i++)
	{
		seaweedX[i] = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * SEAWEED_FIELD_HALF;
		seaweedZ[i] = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * SEAWEED_FIELD_HALF;
		seaweedScale[i] = 0.6f + 0.8f * (float)rand() / RAND_MAX;          // 0.6 to 1.4 x height
		seaweedPhase[i] = 6.28f * (float)rand() / RAND_MAX;                // so they don't all sway together
	}
}

// Same pattern as createTreeDisplayList in the example.
void createSeaweedDisplayList(void)
{
	seaweedDL = glGenLists(1);
	glNewList(seaweedDL, GL_COMPILE);
	drawSeaweedStrand();
	glEndList();
}

// Flat triangle with its normal computed from the corners.
static void flatTri(const float a[3], const float b[3], const float c[3])
{
	float ux = b[0] - a[0], uy = b[1] - a[1], uz = b[2] - a[2];
	float vx = c[0] - a[0], vy = c[1] - a[1], vz = c[2] - a[2];
	glNormal3f(uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx);
	glVertex3fv(a); glVertex3fv(b); glVertex3fv(c);
}

// One leaf: two triangles, drawn on both sides so it's visible from any angle.
// Leaf points along local +X and curls up toward +Z.
static void drawVineLeaf(void)
{
	float s = SEAWEED_LEAF_SCALE;
	float base[3] = { 0, 0, 0 };
	float mL[3] = { 0.45f * s,  0.15f * s, 0.10f * s };
	float mR[3] = { 0.45f * s, -0.15f * s, 0.10f * s };
	float tip[3] = { 0.95f * s, 0, 0.45f * s };

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, VINE_LEAF);
	glBegin(GL_TRIANGLES);
	flatTri(base, mL, mR);  flatTri(base, mR, mL);   // front and back
	flatTri(mL, tip, mR);   flatTri(mL, mR, tip);
	glEnd();
}

// A small cluster of pods hanging off the stalk on thin stems.
static void drawVinePods(void)
{
	int p;
	for (p = 0; p < 3; p++)
	{
		glPushMatrix();
		glRotated(p * 120.0, 0, 0, 1);
		glRotated(90, 0, 1, 0);                      // Point the stem outward (+X).

		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, VINE_STALK);
		gluCylinder(seaweedQuadric, 0.02, 0.02, 0.25, 3, 1);   // Stem.

		glTranslated(0, 0, 0.25);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, VINE_POD);
		gluSphere(seaweedQuadric, 0.12, 6, 4);       // Low-poly pod.
		glPopMatrix();
	}
}

void drawSeaweedStrand(void)
{
	int s;
	double segLen = SEAWEED_HEIGHT / SEAWEED_SEGMENTS;
	double r0, r1;

	glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);

	glPushMatrix();
	glRotated(-90, 1, 0, 0);   // Stalk grows along +Z; this tips it up to +Y.

	for (s = 0; s < SEAWEED_SEGMENTS; s++)
	{
		r0 = SEAWEED_RADIUS * (1.0 - 0.6 * s / SEAWEED_SEGMENTS);
		r1 = SEAWEED_RADIUS * (1.0 - 0.6 * (s + 1) / SEAWEED_SEGMENTS);

		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, VINE_STALK);
		gluCylinder(seaweedQuadric, r0, r1, segLen, SEAWEED_SIDES, 1);

		// Leaves: skip the bare bottom segment, two opposite leaves per node,
		// each node twisted so they spiral up the stalk.
		if (s >= 1)
		{
			glPushMatrix();
			glRotated(s * 137.5, 0, 0, 1);
			glPushMatrix(); glTranslated(r1, 0, 0);  drawVineLeaf(); glPopMatrix();
			glRotated(180, 0, 0, 1);
			glPushMatrix(); glTranslated(r1, 0, 0);  drawVineLeaf(); glPopMatrix();
			glPopMatrix();
		}

		// Pods on every second node, from the middle upward.
		if (s >= 3 && s % 2 == 1)
		{
			glPushMatrix();
			glRotated(s * 60.0, 0, 0, 1);
			drawVinePods();
			glPopMatrix();
		}

		glTranslated(0, 0, segLen);
		glRotated(SEAWEED_BEND, 1, 0, 0);
		glRotated(SEAWEED_BEND * 0.6, 0, 1, 0);   // Slight twist so it isn't a flat curve.
	}
	glPopMatrix();
}


// Replays the list at each position, adding sway outside the list.
void drawSeaweed(void)
{
	int i;
	glShadeModel(GL_FLAT);

	for (i = 0; i < SEAWEED_COUNT; i++)
	{
		glPushMatrix();
		glTranslated(seaweedX[i], seabedHeight(seaweedX[i], seaweedZ[i]) - 0.1f, seaweedZ[i]);
		glRotated(SEAWEED_SWAY_DEG * sinf(seaweed_phase + seaweedPhase[i]), 1, 0, 0);
		glRotated(SEAWEED_SWAY_DEG * 0.5 * sinf(2.0f * seaweed_phase + seaweedPhase[i]), 0, 0, 1);
		glScaled(1.0, seaweedScale[i], 1.0);
		glCallList(seaweedDL);
		glPopMatrix();
	}
	glShadeModel(smoothOn ? GL_SMOOTH : GL_FLAT);

}

/****************************************2026****************************************/