/***********************************************************
             CSC418/2504, Fall 2011
  
                 robot.cpp
                 
       Simple demo program using OpenGL and the glut/glui 
       libraries

  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design.

        Add source code where it appears appropriate. In
        particular, see lines marked 'TODO'.

        You should not need to change the overall structure
        of the program. However it should be clear what
        your changes do, and you should use sufficient comments
        to explain your code.  While the point of the assignment
        is to draw and animate the character, you will
        also be marked based on your design.
		
		Some windows-specific code remains in the includes; 
		we are not maintaining windows build files this term, 
		but we're leaving that here in case you want to try building
		on windows on your own.

***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef _WIN32
#include <unistd.h>
#else
void usleep(unsigned int nanosec)
{
    Sleep(nanosec / 1000);
}
#endif


// *************** GLOBAL VARIABLES *************************


const float PI = 3.14159;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;               // Glut window ID (for display)
GLUI *glui;                 // Glui window (for controls)
int Win[2];                 // window (x,y) size


// ---------------- POLYGON VARIABLES -----------------------
// Head
const float HEAD_HEIGHT = 40;
const float FACE_HEIGHT = 30;
const float UPPER_WIDTH = 30;
const float LOWER_WIDTH = 50;
// Mouth
const float MOUTH_LENGTH = 28;
const float MOUTH_TIP = 5;
const float MOUTH_BACK = 10;
// Beak
const float BEAK_LENGTH = 28;
const float BEAK_WIDTH = 5;
// Body
const float BODY_HEIGHT = 90;
const float BUTT_HEIGHT = 20;
const float BODY_UPPER_WIDTH = 30;
const float BODY_BUTT_WIDTH = 60;
const float BODY_BOTTOM_WIDTH = 15;
// Joint
const float JOINT_OFFSET = 5;
// Wing
const float WING_HEIGHT = 40;
const float WING_UPPER_WIDTH = 20;
const float WING_LOWER_WIDTH = 10;
// Leg
const float LEG_HEIGHT = 30;
const float LEG_WIDTH = 10;
// Foot
const float FOOT_WIDTH = 8;
const float FOOT_LENGTH = 25;


// ---------------- ANIMATION VARIABLES ---------------------

// Animation settings
int animate_mode = 0;       // 0 = no anim, 1 = animate
int animation_frame = 0;      // Specify current frame of animation

// Joint parameters
const float JOINT_MIN = -45.0f;
const float JOINT_MAX =  45.0f;
float joint_rot = 0.0f;

//////////////////////////////////////////////////////
// TODO: Add additional joint parameters here
//////////////////////////////////////////////////////



// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initGlut(char* winName);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void myReshape(int w, int h);
void animate();
void display(void);

// Callback for handling events in glui
void GLUI_Control(int id);


// Functions to help draw the object
void drawSquare(float size);
void drawBody();
void drawHead();
void drawTrapzoid();
void drawMouth();


// Return the current system clock (in seconds)
double getTime();


// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 300x200 window by default...\n");
        Win[0] = 300;
        Win[1] = 200;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize glut, glui, and opengl
    glutInit(&argc, argv);
    initGlut(argv[0]);
    initGlui();
    initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Initialize glut and create a window with the specified caption 
void initGlut(char* winName)
{
    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(winName);

    // Setup callback functions to handle events
    glutReshapeFunc(myReshape); // Call myReshape whenever window resized
    glutDisplayFunc(display);   // Call display whenever new frame needed 
}


// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Animate button handler.  Called when the "animate" checkbox is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui->sync_live();

  animation_frame = 0;
  if(animate_mode == 1) {
    // start animation
    GLUI_Master.set_glutIdleFunc(animate);
  } else {
    // stop animation
    GLUI_Master.set_glutIdleFunc(NULL);
  }
}

// Initialize GLUI and the user interface
void initGlui()
{
    GLUI_Master.set_glutIdleFunc(NULL);

    // Create GLUI window
    glui = GLUI_Master.create_glui("Glui Window", 0, Win[0]+10, 0);

    // Create a control to specify the rotation of the joint
    GLUI_Spinner *joint_spinner
        = glui->add_spinner("Joint", GLUI_SPINNER_FLOAT, &joint_rot);
    joint_spinner->set_speed(0.1);
    joint_spinner->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    ///////////////////////////////////////////////////////////
    // TODO: 
    //   Add controls for additional joints here
    ///////////////////////////////////////////////////////////

    // Add button to specify animation mode 
    glui->add_separator();
    glui->add_checkbox("Animate", &animate_mode, 0, animateButton);

    // Add "Quit" button
    glui->add_separator();
    glui->add_button("Quit", 0, quitButton);

    // Set the main window to be the "active" window
    glui->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}




// Callback idle function for animating the scene
void animate()
{
    // Update geometry
    const double joint_rot_speed = 0.1;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
    
    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function animate the character's joints
    //   Note: Nothing should be drawn in this function!  OpenGL drawing
    //   should only happen in the display() callback.
    ///////////////////////////////////////////////////////////

    // Update user interface
    glui->sync_live();

    // Tell glut window to update itself.  This will cause the display()
    // callback to be called, which renders the object (once you've written
    // the callback).
    glutSetWindow(windowID);
    glutPostRedisplay();

    // increment the frame number.
    animation_frame++;

    // Wait 50 ms between frames (20 frames per second)
    usleep(50000);
}


// Handles the window being resized by updating the viewport
// and projection matrices
void myReshape(int w, int h)
{
    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-w/2, w/2, -h/2, h/2);

    // Update OpenGL viewport and internal variables
    glViewport(0,0, w,h);
    Win[0] = w;
    Win[1] = h;
}



// display callback
//
// This gets called by the event handler to draw
// the scene, so this is where you need to build
// your scene -- make your changes and additions here.
// All rendering happens in this function.  For Assignment 1,
// updates to geometry should happen in the "animate" function.
void display(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);

    // OK, now clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function draw the scene
    //   This should include function calls to pieces that
    //   apply the appropriate transformation matrice and
    //   render the individual body parts.
    ///////////////////////////////////////////////////////////

    // Draw our hinged object
    const float BODY_WIDTH = 30.0f;
    const float BODY_LENGTH = 50.0f;
    const float ARM_LENGTH = 50.0f;
    const float ARM_WIDTH = 10.0f;

    // Push the current transformation matrix on the stack
    glPushMatrix();
        
    // Draw the 'body'
    glPushMatrix();
    // Set the colour to green
    glColor3f(0.0, 1.0, 0.0);
    drawBody();
    glPopMatrix();
    
    /**
     ** Draw the head+mouth system
     **/
    glPushMatrix();
    
    // Move to the joint
    glTranslatef(0.0, BODY_HEIGHT/2 - JOINT_OFFSET, 0.0);
    // Rotate on the joint
    glRotatef(joint_rot, 0.0, 0.0, 1.0);
    
    // First draw the head
    glPushMatrix();
    // Move to the location of the head
    glTranslatef(0.0, 13, 0.0);
    glColor3f(1.0, 0.0, 0.0);
    drawHead();
    glPopMatrix();
    
    // Second draw the mouth
    glPushMatrix();
    // Move to the mouth location
    glTranslatef(-UPPER_WIDTH, MOUTH_BACK+MOUTH_TIP, 0.0);
    glColor3f(0.0, 0.0, 0.0);
    drawMouth();
    glPopMatrix();
    
    // Third draw the beak
    glPushMatrix();
    // Move to the beak location
    glTranslatef(-UPPER_WIDTH, 5, 0.0);
    glScalef(BEAK_LENGTH, BEAK_WIDTH, 0.0);
    glColor3f(0.0, 0.0, 0.0);
    drawSquare(1.0);
    glPopMatrix();
    
    glPopMatrix();
    /** End of head+mouth system **/
    
    // Draw the wing
    glPushMatrix();
    // Move to the joint
    glTranslatef(0.0, BODY_HEIGHT/4, 0.0);
    // Rotate on the joint
    glRotatef(joint_rot, 0.0, 0.0, 1.0);
    // Move the center
    glTranslatef(0.0, -WING_HEIGHT/2+JOINT_OFFSET, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    drawTrapzoid();
    glPopMatrix();
    
    /**
     ** Draw the left leg+foot system
     **/
    glPushMatrix();
    
    // Translate to the joint of the leg
    glTranslatef(-BODY_BUTT_WIDTH/5-JOINT_OFFSET, -BUTT_HEIGHT-8, 0.0);
    // Rotate on the joint for the entire system
    glRotatef(joint_rot, 0.0, 0.0, 1.0);
    
    // First draw the leg
    glPushMatrix();
    // Translate to leg's position
    glTranslatef(0.0, -LEG_HEIGHT/3, 0.0);
    // Scale the size of the leg
    glScalef(LEG_WIDTH, LEG_HEIGHT, 1.0);
    glColor3f(1.0, 0.0, 0.0);
    drawSquare(1.0);
    glPopMatrix();
    
    // Second draw the foot
    glPushMatrix();
    // Translate to the joint of the foot
    glTranslatef(0.0, -LEG_HEIGHT+2*JOINT_OFFSET, 0.0);
    // Foot's rotation on the foot's joint
    glRotatef(joint_rot, 0.0, 0.0, 1.0);
    // Now apply foot's indiviual translation
    glPushMatrix();
    glTranslatef(-FOOT_LENGTH/2+JOINT_OFFSET, 0.0, 0.0);
    // Scale to the foot size
    glScalef(FOOT_LENGTH, FOOT_WIDTH, 1.0);
    glColor3f(0.0, 0.5, 0.5);
    drawSquare(1.0);
    glPopMatrix();
    glPopMatrix();
    
    glPopMatrix();
    /** End of left leg+foot system **/
    
    /**
     ** Draw the right leg+foot system
     **/
    glPushMatrix();
    
    // Translate to the joint of the leg
    glTranslatef(BODY_BUTT_WIDTH/5+JOINT_OFFSET, -BUTT_HEIGHT-8, 0.0);
    // Rotate on the joint for the entire system
    glRotatef(joint_rot, 0.0, 0.0, 1.0);
    
    // First draw the leg
    glPushMatrix();
    // Translate to leg's position
    glTranslatef(0.0, -LEG_HEIGHT/3, 0.0);
    // Scale the size of the leg
    glScalef(LEG_WIDTH, LEG_HEIGHT, 1.0);
    glColor3f(1.0, 0.0, 0.0);
    drawSquare(1.0);
    glPopMatrix();
    
    // Second draw the foot
    glPushMatrix();
    // Translate to the joint of the foot
    glTranslatef(0.0, -LEG_HEIGHT+2*JOINT_OFFSET, 0.0);
    // Foot's rotation on the foot's joint
    glRotatef(joint_rot, 0.0, 0.0, 1.0);
    // Now apply foot's indiviual translation
    glPushMatrix();
    glTranslatef(-FOOT_LENGTH/2+JOINT_OFFSET, 0.0, 0.0);
    // Scale to the foot size
    glScalef(FOOT_LENGTH, FOOT_WIDTH, 1.0);
    glColor3f(0.0, 0.5, 0.5);
    drawSquare(1.0);
    glPopMatrix();
    glPopMatrix();
    
    glPopMatrix();
    /** End of right leg+foot system **/
    
    
    // Retrieve the previous state of the transformation stack
    glPopMatrix();


    // Execute any GL functions that are in the queue just to be safe
    glFlush();

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}


//Draw the body, centered at the current location
void drawBody() {
    glBegin(GL_POLYGON);
    glVertex2f(-BODY_UPPER_WIDTH/2, BODY_HEIGHT/2);
    glVertex2f(BODY_UPPER_WIDTH/2, BODY_HEIGHT/2);
    glVertex2f(BODY_BUTT_WIDTH/2, -(BODY_HEIGHT/2 - BUTT_HEIGHT));
    glVertex2f(BODY_BOTTOM_WIDTH/2, -BODY_HEIGHT/2);
    glVertex2f(-BODY_BOTTOM_WIDTH/2, -BODY_HEIGHT/2);
    glVertex2f(-BODY_BUTT_WIDTH/2, -(BODY_HEIGHT/2 - BUTT_HEIGHT));
    glEnd();
}


//Draw the head, centered at the current location
void drawHead() {
    glBegin(GL_POLYGON);
    glVertex2f(-5, HEAD_HEIGHT/2);
    glVertex2f(UPPER_WIDTH/2, FACE_HEIGHT/2);
    glVertex2f(LOWER_WIDTH/2, -FACE_HEIGHT/2);
    glVertex2f(-LOWER_WIDTH/2, -FACE_HEIGHT/2);
    glVertex2f(-UPPER_WIDTH/2, FACE_HEIGHT/2);
    glEnd();
}

// Draw the mouth
void drawMouth() {
    glBegin(GL_POLYGON);
    glVertex2f(-MOUTH_LENGTH/2, 0);
    glVertex2f(MOUTH_LENGTH/2, MOUTH_BACK/2);
    glVertex2f(MOUTH_LENGTH/2, -MOUTH_BACK/2);
    glVertex2f(-MOUTH_LENGTH/2, -MOUTH_TIP);
    glEnd();
}


// Draw the wing
void drawTrapzoid() {
    glBegin(GL_POLYGON);
    glVertex2f(-WING_UPPER_WIDTH/2, WING_HEIGHT/2);
    glVertex2f(WING_UPPER_WIDTH/2, WING_HEIGHT/2);
    glVertex2f(WING_LOWER_WIDTH/2, -WING_HEIGHT/2);
    glVertex2f(-WING_LOWER_WIDTH/2, -WING_HEIGHT/2);
    glEnd();
}


// Draw a square of the specified size, centered at the current location
void drawSquare(float width)
{
    // Draw the square
    glBegin(GL_POLYGON);
    glVertex2d(-width/2, -width/2);
    glVertex2d(width/2, -width/2);
    glVertex2d(width/2, width/2);
    glVertex2d(-width/2, width/2);
    glEnd();
}

