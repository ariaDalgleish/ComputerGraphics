/******************************************************************************
 *
 * Animation v2.0 (15/07/2026)
 *
 * This template provides a basic FPS-limited render loop for an animated scene.
 *
 ******************************************************************************/

#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

 /******************************************************************************
  * Particles set up structs
  ******************************************************************************/
  // purpose of this struct is to hold the position of a particle in 2D space (x and y coordinates)
typedef struct {
	float x;
	float y;
} Position2;

// purpose of this struct is to hold the properties of a particle, including its position, size, velocity, age, lifetime, and active status
typedef struct {
	Position2 position; // x y location of particle
	float size;
	float vx, vy;
	float age;       // seconds alive
	float lifetime;  // seconds to live
	int active; // 0 = inactive

} Particle_t;

#define MAX_PARTICLES 1000

static Particle_t particles[MAX_PARTICLES];
/******************************************************************************
  * Global Particle Variables
  ******************************************************************************/
float particleGravity = 0.0f;        // used in update_particles() negative = downward
float particleInitVyScale = 0.20f;     // used in spawn_particle() controls initial downward speed
float particleInitVxScale = 0.05f;     // used in spawn_particle() controls horizontal spread
float spawnRate = 10.0f;           // particles/sec (start)
float spawnAccumulator = 0.0f;	// used to track time between spawns
float spawnGrowthPerSec = 0.5f;  // particles/sec^2 (increase spawn rate over time)
float spawnRateMax = 12.0f;       // cap

void particles_init(void);
void spawn_particles(int count);
void particles_update(float dt); // dt = delta time in seconds since last update
void particles_draw(void);
void particle_test_draw(void);
void drawBackgroundGradient(void);

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
 * Keyboard Input Handling Setup
 ******************************************************************************/

 // Define all character keys used for input (add any new key definitions here).
 // Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
 // characters typed by the user to lowercase, so the SHIFT key is ignored.

#define KEY_EXIT			27 // Escape key.

int renderFillEnabled = 1;

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void idle(void);

/******************************************************************************
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/

void main(int argc, char** argv);
void init(void);
void think(void);

/******************************************************************************
 * Animation-Specific Setup (Add your own definitions, constants, and globals here)
 ******************************************************************************/

 /******************************************************************************
  * Entry Point (don't put anything except the main function here)
  ******************************************************************************/

void main(int argc, char** argv)
{
	// Initialize the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Animation");

	// Set up the scene.
	init();

	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
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
	if (!renderFillEnabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	//: REPLACE THIS COMMENT WITH YOUR DRAWING CODE
		/* Drawing: clear, draw test particle, draw active particles */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawBackgroundGradient();
	//Separate reusable pieces of drawing code into functions, which you can add
	//to the "Animation-Specific Functions" section below.

/* Basic modelview/projection reset (simple 2D drawing in clip space) */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Remember to add prototypes for any new functions to the "Animation-Specific
	//Function Prototypes" section near the top of this template.

/* Draw a single test particle at a known location*/
	particle_test_draw();
	/* Draw all active particles in the particle system */
	particles_draw();
	/* Present the rendered image */
	glutSwapBuffers();

}

/*
	Called when the OpenGL window has been resized.
*/
void reshape(int width, int h)
{
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is pressed.
*/
void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {
		/*
			TEMPLATE: Add any new character key controls here.

			Rather than using literals (e.g. "d" for diagnostics), create a new KEY_
			definition in the "Keyboard Input Handling Setup" section of this file.
		*/
	case 'l':
		renderFillEnabled = !renderFillEnabled;
		break;
	case KEY_EXIT:
		exit(0);
		break;
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
	/* Basic GL setup for point rendering and random seed */
	glPointSize(1.0f);
	glEnable(GL_POINT_SMOOTH);
	// GL_POINT_SMOOTH_HINT is a hint to OpenGL about the quality of point smoothing.
	// GL_NICEST indicates that we want the highest quality smoothing,
	// which may be slower but will produce better visual results.
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	// enables lighting calculations for 3D objects.
	glDisable(GL_LIGHTING);

	// Seed the random number generator for particle randomness.
	srand((unsigned int)time(NULL));
	particles_init(); // Initialize the particle system.
}

/*
	Advance our animation by FRAME_TIME milliseconds.

	Note: Our template's GLUT idle() callback calls this once before each new
	frame is drawn, EXCEPT the very first frame drawn after our application
	starts. Any setup required before the first frame is drawn should be placed
	in init().
	Advance our animation by FRAME_TIME seconds (FRAME_TIME_SEC)
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
	/* Spawn a few new particles each frame for demonstration */
	//spawn_particles(6);
	/* Update particle dynamics using fixed timestep in seconds */
	//particles_update(FRAME_TIME_SEC);

	spawnAccumulator += spawnRate * FRAME_TIME_SEC;          // accumulate fractional particles
	int toSpawn = (int)spawnAccumulator;
	if (toSpawn > 0) {
		spawn_particles(toSpawn);
		spawnAccumulator -= toSpawn;
	}
	/* subtle growth */

	spawnRate += spawnGrowthPerSec * FRAME_TIME_SEC;
	if (spawnRate > spawnRateMax) spawnRate = spawnRateMax;
	particles_update(FRAME_TIME_SEC);
}

/* Initialise the particle system by marking all particles as inactive */
void particles_init(void)
{
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		particles[i].active = 0;
		particles[i].age = 0.0f;
	}
}

/* Spawn up to 'count' new particles by recycling inactive slots*/
void spawn_particles(int count)
{
	for (int c = 0; c < count; ++c) {
		for (int i = 0; i < MAX_PARTICLES; ++i) {
			if (!particles[i].active) {
				/* initialize attributes (positions in clip-space [-1,1]) */
				particles[i].position.x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
				particles[i].position.y = 1.0f; // spawn top of screen
				particles[i].vx = ((float)rand() / RAND_MAX) * 2.0f * particleInitVxScale - particleInitVxScale; // random horizontal velocity
				particles[i].vy = -((float)rand() / RAND_MAX) * particleInitVyScale - 0.1f; // random downward velocity

				
				//particles[i].vx = ((float)rand() / RAND_MAX) * 1.0f - 0.5f; // horizontal spread
				//particles[i].vy = -((float)rand() / RAND_MAX) * 0.1f - 0.5f; // initial downward velocity
				particles[i].size = ((float)rand() / RAND_MAX) * 3.0f + 1.5f; // random size between 2.0 and 5.0
				particles[i].age = 0.0f;
				particles[i].lifetime = 5.0f + ((float)rand() / RAND_MAX) * 10.0f; // 1..11s
				particles[i].active = 1;
				break; /* spawn one particle per call to this function */
			}
		}
	}
}

void particles_update(float dt)
{
	const float gravity = particleGravity;
	//const float gravity = -0.98f; /* units per second^2 (tuned for demo) */
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		if (!particles[i].active) continue;

		particles[i].age += dt;
		if (particles[i].age > particles[i].lifetime) {
			particles[i].active = 0; /* deactivate for reuse*/
			continue;
		}
		/* Integrate velocity/position */
		particles[i].vy += gravity * dt;
		particles[i].position.x += particles[i].vx * dt; // update position based on velocity
		particles[i].position.y += particles[i].vy * dt; 

		/* Extinction: out of view*/ 
		if (particles[i].position.y < -1.5f ||
			particles[i].position.x < -2.0f ||
			particles[i].position.x > 2.0f) {
			particles[i].active = 0;
		}
	}
}


/* Draw all active particles as GL_POINTS */
void particles_draw(void)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		if (!particles[i].active) continue;
		/* size and color vary with age */
		float t = particles[i].age / particles[i].lifetime;
		float alpha = 0.8f - t;
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
		glPointSize(particles[i].size);
		glBegin(GL_POINTS);
		glVertex2f(particles[i].position.x, particles[i].position.y);
		glEnd();
	}
	glDisable(GL_BLEND);
}

/* Test draw: create a known particle and draw as a point primitive. Called from display() */
void particle_test_draw(void)
{
	/* Known position in clip-space */
	float tx = 0.0f;
	float ty = 0.0f;
	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(8.0f);
	glBegin(GL_POINTS);
	glVertex2f(tx, ty);
	glEnd();
}

void drawBackgroundGradient(void)
{
	/* Draw a full-screen quad with interpolated colors (top->bottom gradient).
	   Disable depth write/test so it doesn't occlude scene geometry. */
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	/* Preserve and reset projection/modelview so quad covers clip-space [-1,1]. */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); glLoadIdentity();

	glBegin(GL_QUADS);
	/* top (lighter) */
	glColor3f(0.161f, 0.596f, 0.776f); glVertex2f(-1.0f, 1.0f);
	glColor3f(0.161f, 0.596f, 0.776f); glVertex2f(1.0f, 1.0f);
	/* bottom (darker) */
	glColor3f(0.51f, 0.784f, 0.898f); glVertex2f(1.0f, -1.0f);
	glColor3f(0.51f, 0.784f, 0.898f); glVertex2f(-1.0f, -1.0f);
	glEnd();

	/* restore matrices */
	glPopMatrix(); /* MODELVIEW */
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	/* restore depth state */
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}


/**************************************2026*S2****************************************/