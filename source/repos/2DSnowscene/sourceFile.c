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

#define GROUND_VERTS 5
typedef struct { float x, y; } Vec2;
static Vec2 groundTop[GROUND_VERTS];




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

int windowHeight = 800;
int windowWidth = 800;

static float groundJitter[GROUND_VERTS]; // 0..1

float snowmanX = 200.0f;     // fixed pixel x (tweak)
float snowmanY = 300.0f;     // will be set in init()
float snowmanBaseR = 60.0f;  // fixed radius
float snowmanT = 0.25f;      // normalized horizontal position (0..1) on the ground silhouette


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

#define KEY_EXIT 'q' //Exit key (q).

int renderFillEnabled = 1;

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void idle(void);
void mouseClicked(int button, int state, int x, int y);

/******************************************************************************
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/

void main(int argc, char** argv);
void init(void);
void think(void);

/******************************************************************************
 * Animation-Specific Setup (Add your own definitions, constants, and globals here)
 ******************************************************************************/

void particles_init(void);
void spawn_particles(int count);
void particles_update(float dt); // dt = delta time in seconds since last update
void particles_draw(void);
void particle_test_draw(void);
void drawBackgroundGradient(void);

void generate_ground(void);
void draw_ground(void);

/* Snowman helpers */
void drawFilledCircleLit(float cx, float cy, float radius, int segments, float lightDirX, float lightDirY);
void drawSnowman(float cx, float cy, float baseRadius);
float getGroundHeightAtX(float x);
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
	glutMouseFunc(mouseClicked);
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

	/* Drawing: clear, draw background, particles, and eyes */
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawBackgroundGradient();
	draw_ground();

	float avgGroundY = 0.0f;
	for (int i = 0;i < GROUND_VERTS;i++) avgGroundY += groundTop[i].y;
	avgGroundY /= GROUND_VERTS;

	glDisable(GL_DEPTH_TEST);
	drawSnowman(snowmanX, snowmanY, snowmanBaseR);
	glEnable(GL_DEPTH_TEST);

	/* then draw particles (they'll all be in front for now) */
	particle_test_draw();
	particles_draw();

	glutSwapBuffers();
}

/*
	Called when the OpenGL window has been resized.
*/
void rebuild_ground_positions(void) {
	float topBase = windowHeight / 4.0f, amp = topBase * 0.30f;
	for (int i = 0; i < GROUND_VERTS; i++) {
		float t = (float)i / (GROUND_VERTS - 1);
		groundTop[i].x = t * windowWidth;
		groundTop[i].y = topBase - groundJitter[i] * amp;
	}
	/* Keep snowman attached to the ground silhouette by using a normalized X (snowmanT)
	   snowmanX is recomputed to match the current windowWidth, and snowmanY is set
	   slightly below the top so the snowman appears 'in' the ground. */
	snowmanX = snowmanT * windowWidth;
	{
		float groundY = getGroundHeightAtX(snowmanX);
		/* embed half the base into the ground */
		snowmanY = groundY - (snowmanBaseR * 0.5f);
		/* clamp so snowman stays visible */
		if (snowmanY < snowmanBaseR) snowmanY = snowmanBaseR;
		if (snowmanY + snowmanBaseR * 3.0f > windowHeight) snowmanY = windowHeight - snowmanBaseR * 3.0f;
	}
}

void reshape(int width, int h)
{
	windowWidth = width;
	windowHeight = h;
	glViewport(0, 0, width, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (float)width, 0.0, (float)h);
	glMatrixMode(GL_MODELVIEW);
	rebuild_ground_positions();
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

// Called when a mouse button is pressed. Left click places the snowman on the ground.
void mouseClicked(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		float sx = (float)x;
		if (sx < 0.0f) sx = 0.0f; if (sx > (float)windowWidth) sx = (float)windowWidth;
		// store normalized position so snowman stays attached when window resizes
		snowmanT = sx / (float)windowWidth;
		snowmanX = sx;
		float groundY = getGroundHeightAtX(snowmanX);
		// embed half the base into the ground
		snowmanY = groundY - (snowmanBaseR * 0.5f);
		if (snowmanY < snowmanBaseR) snowmanY = snowmanBaseR;
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
	generate_ground();

	/* Choose a normalized horizontal position for the snowman so it stays
	   attached to the ground if the window is resized. 0.0 = left, 1.0 = right. */
	snowmanT = 0.25f; // 25% across the screen; tweak as desired
	snowmanBaseR = 60.0f;
	/* compute pixel X/Y from normalized position and embedding into ground */
	snowmanX = snowmanT * windowWidth;
	{
		float groundY = getGroundHeightAtX(snowmanX);
		/* embed half the base into the ground */
		snowmanY = groundY - (snowmanBaseR * 0.5f);
		if (snowmanY < snowmanBaseR) snowmanY = snowmanBaseR;
	}

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

void generate_ground(void) {
	for (int i = 0;i < GROUND_VERTS;i++) groundJitter[i] = (float)rand() / RAND_MAX;
	rebuild_ground_positions();
}

void draw_ground(void)
{
	const float topR = 1.0f, topG = 1.0f, topB = 1.0f;          // bright snow
	const float bottomR = 0.85f, bottomG = 0.86f, bottomB = 0.88f; // base grey
	const float baseAlpha = 1.0f;
	const float topAlpha = 0.2f; // 0 = fully faded into background

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < GROUND_VERTS; ++i) {
		float x = groundTop[i].x, y = groundTop[i].y;
		// bottom vertex (opaque)
		glColor4f(bottomR, bottomG, bottomB, baseAlpha);
		glVertex2f(x, 0.0f);
		// compute normalized t (0 at lowest possible top, 1 at highest top baseline)
		float t = (y - (windowHeight / 8.0f)) / (windowHeight / 4.0f);
		if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
		float r = bottomR + (topR - bottomR) * t;
		float g = bottomG + (topG - bottomG) * t;
		float b = bottomB + (topB - bottomB) * t;
		float a = baseAlpha + (topAlpha - baseAlpha) * t; // fades toward topAlpha
		glColor4f(r, g, b, a);
		glVertex2f(x, y);
	}
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

// draw a filled, lit circle using a triangle fan. lightDir gives lighting direction in 2D.
void drawFilledCircleLit(float cx, float cy, float radius, int segments, float lightDirX, float lightDirY)
{
	// normalize light dir
	float ldlen = sqrtf(lightDirX * lightDirX + lightDirY * lightDirY);
	if (ldlen == 0.0f) { lightDirX = 0.0f; lightDirY = 1.0f; ldlen = 1.0f; }
	lightDirX /= ldlen; lightDirY /= ldlen;

	glBegin(GL_TRIANGLE_FAN);
	// center is brightest (snow highlight)
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(cx, cy);

	for (int i = 0; i <= segments; ++i) {
		float a = (2.0f * 3.14159265359f * i) / segments;
		float sx = cosf(a), sy = sinf(a);
		float vx = cx + sx * radius;
		float vy = cy + sy * radius;

		// approximate lighting: dot between outward normal (sx,sy) and light dir
		float nd = sx * lightDirX + sy * lightDirY;
		if (nd < 0.0f) nd = 0.0f;
		// map nd to brightness in [0.7,1.0] for subtle shading
		float bright = 0.7f + 0.3f * nd;
		glColor3f(bright, bright, bright);

		glVertex2f(vx, vy);
	}
	glEnd();
}

// Draw a 3-ball snowman. (cx,cy) = center of bottom (base) circle; baseRadius scales whole snowman.
void drawSnowman(float cx, float cy, float baseRadius)
{
	// overlap: 0.0 = just touching, 0.5 = 50% overlap
	float overlapMiddle = 0.45f; // tune 0.15..0.40
	float overlapHead = 0.15f; // generally larger overlap for smaller head

	// relative radii (tweak ratios as you like)
	float r1 = baseRadius;           // base
	float r2 = baseRadius * 0.82f;   // middle
	float r3 = baseRadius * 0.62f;   // head

	float c1x = cx, c1y = cy;
	float c2x = cx;
	float c2y = c1y + r1 + r2 - overlapMiddle * r2;
	float c3x = cx;
	float c3y = c2y + r2 + r3 - overlapHead * r3;

	// light direction (from top-left)
	float lx = -0.5f, ly = 0.8f;

	// draw balls: largest first (back-to-front)
	drawFilledCircleLit(c1x, c1y, r1, 48, lx, ly);
	drawFilledCircleLit(c2x, c2y, r2, 40, lx, ly);
	drawFilledCircleLit(c3x, c3y, r3, 36, lx, ly);

	// optional: add simple eyes/nose/buttons with small filled circles/triangles
	// e.g., glColor3f(0,0,0); draw small points for eyes on the head, etc.
}
/* Sample ground height at a given screen X. Linearly interpolates between top vertices. */
float getGroundHeightAtX(float x)
{
	if (GROUND_VERTS < 2) return windowHeight / 4.0f;
	if (x <= groundTop[0].x) return groundTop[0].y;
	if (x >= groundTop[GROUND_VERTS - 1].x) return groundTop[GROUND_VERTS - 1].y;
	for (int i = 0; i < GROUND_VERTS - 1; ++i) {
		float x0 = groundTop[i].x, x1 = groundTop[i + 1].x;
		if (x >= x0 && x <= x1) {
			float t = (x - x0) / (x1 - x0);
			return groundTop[i].y * (1.0f - t) + groundTop[i + 1].y * t;
		}
	}
	return windowHeight / 4.0f;
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
				/* initialize attributes in screen-space (0 to windowWidth/Height) */
				particles[i].position.x = ((float)rand() / RAND_MAX) * windowWidth;
				particles[i].position.y = (float)windowHeight; // spawn at top of screen
				//particles[i].vx = ((float)rand() / RAND_MAX) * 2.0f * particleInitVxScale - particleInitVxScale; // random horizontal velocity
				//particles[i].vy = -((float)rand() / RAND_MAX) * particleInitVyScale - 0.1f; // random downward velocity
				particles[i].vx = ((float)rand() / RAND_MAX) * 100.0f * particleInitVxScale - 50.0f * particleInitVxScale;
				particles[i].vy = -((float)rand() / RAND_MAX) * 150.0f * particleInitVyScale - 75.0f; // downward velocity


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
	//const float gravity = particleGravity;
	const float gravity = particleGravity * 300.0f; // scale gravity for screen-space (pixels/sec²)
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
		if (particles[i].position.y < 0.0f ||
			particles[i].position.x < -100.0f ||
			particles[i].position.x > windowWidth + 100.0f) {
			particles[i].active = 0;
			/*if (particles[i].position.y < -1.5f ||
				particles[i].position.x < -2.0f ||
				particles[i].position.x > 2.0f) {
				particles[i].active = 0;*/
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

/**************************************2026*S2****************************************/