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
const float EYE_RADIUS = 5;
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
const float JOINT_OFFSET = 3;
const float JOINT_RADIUS = 2.5;
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
const float BEAK_MAX = 10;
const float BEAK_MIN = 5;
const float WING_MAX = 1;
const float WING_MIN = 0.1;
const float BODY_X_MIN = -40;
const float BODY_X_MAX = 40;
const float BODY_Y_MIN = -30;
const float BODY_Y_MAX = 30;
float beak_y = BEAK_MIN;
float wing_y = WING_MAX;
float wing_joint_rot = 0.0f;
float left_leg_joint_rot = 0.0f;
float right_leg_joint_rot = 0.0f;
float left_foot_joint_rot = 10.0f;
float right_foot_joint_rot = 10.0f;
float body_x = 0;
float body_y = 0;
bool body_x_moved = false;
bool body_y_moved = false;


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
void drawTrapzoid(float wing_y);
void drawMouth();
void drawCircle(float radius);
void drawColouredCircle(float radius);

// Functions to animate each part
void animateHead();
void animateBeak();
void animateWing();
void animateRightLeg();
void animateRightFoot();
void animateLeftLeg();
void animateLeftFoot();
void animateBody();

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
    
    GLUI_Spinner *joint_spinner[10];
    
    // Control for the head
    joint_spinner[0]
        = glui->add_spinner("Head Joint", GLUI_SPINNER_FLOAT, &joint_rot);
    joint_spinner[0]->set_speed(0.1);
    joint_spinner[0]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    ///////////////////////////////////////////////////////////
    // TODO: 
    //   Add controls for additional joints here
    ///////////////////////////////////////////////////////////
    
    // Control for joint on wing
    joint_spinner[1]
    = glui->add_spinner("Wing Joint", GLUI_SPINNER_FLOAT, &wing_joint_rot);
    joint_spinner[1]->set_speed(0.1);
    joint_spinner[1]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Control for joint on left leg
    joint_spinner[2]
    = glui->add_spinner("Left Leg Joint", GLUI_SPINNER_FLOAT, &left_leg_joint_rot);
    joint_spinner[2]->set_speed(0.1);
    joint_spinner[2]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);
    
    // Control for joint on right leg
    joint_spinner[3]
    = glui->add_spinner("Right Leg Joint", GLUI_SPINNER_FLOAT, &right_leg_joint_rot);
    joint_spinner[3]->set_speed(0.1);
    joint_spinner[3]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);
    
    // Control for joint on left foot
    joint_spinner[4]
    = glui->add_spinner("Left foot Joint", GLUI_SPINNER_FLOAT, &left_foot_joint_rot);
    joint_spinner[4]->set_speed(0.1);
    joint_spinner[4]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);
    
    // Control for joint on right foot
    joint_spinner[5]
    = glui->add_spinner("Right Foot Joint", GLUI_SPINNER_FLOAT, &right_foot_joint_rot);
    joint_spinner[5]->set_speed(0.1);
    joint_spinner[5]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);
    
    // Control for beak
    joint_spinner[6]
    = glui->add_spinner("Beak Height", GLUI_SPINNER_FLOAT, &beak_y);
    joint_spinner[6]->set_speed(0.1);
    joint_spinner[6]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);
    
    // Control for wing scaling
    joint_spinner[7]
    = glui->add_spinner("Wing Scaling", GLUI_SPINNER_FLOAT, &wing_y);
    joint_spinner[7]->set_speed(0.1);
    joint_spinner[7]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);
    
    // Control for body on x-axis
    joint_spinner[8]
    = glui->add_spinner("Body x-axis", GLUI_SPINNER_FLOAT, &body_x);
    joint_spinner[8]->set_speed(0.1);
    joint_spinner[8]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);
    
    // Control for body on y-axis
    joint_spinner[9]
    = glui->add_spinner("Body y-axis", GLUI_SPINNER_FLOAT, &body_y);
    joint_spinner[9]->set_speed(0.1);
    joint_spinner[9]->set_float_limits(JOINT_MIN, JOINT_MAX, GLUI_LIMIT_CLAMP);

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
    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function animate the character's joints
    //   Note: Nothing should be drawn in this function!  OpenGL drawing
    //   should only happen in the display() callback.
    ///////////////////////////////////////////////////////////

    animateHead();
    animateBeak();
    animateWing();
    animateLeftLeg();
    animateLeftFoot();
    animateRightLeg();
    animateRightFoot();
    animateBody();
    
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


void animateHead(){
    // Update geometry
    const double joint_rot_speed = 0.1;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
}


void animateBeak() {
    // Update geometry
    const double joint_rot_speed = 0.13;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    beak_y = joint_rot_t * BEAK_MIN + (1 - joint_rot_t) * BEAK_MAX;

}


void animateWing() {
    // Update geometry
    const double joint_rot_speed = 0.05;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    wing_joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
    
    const double scaling_speed = 0.1;
    joint_rot_t = (sin(animation_frame*scaling_speed) + 1.0) / 2.0;
    wing_y = joint_rot_t * WING_MIN + (1 - joint_rot_t) * WING_MAX;
}

void animateRightFoot() {
    // Update geometry
    const double joint_rot_speed = 0.05;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    right_foot_joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
}


void animateRightLeg() {
    // Update geometry
    const double joint_rot_speed = 0.1;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    right_leg_joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;
}

void animateLeftLeg() {
    // Update geometry
    const double joint_rot_speed = 0.1;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    left_leg_joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;

}

void animateLeftFoot() {
    // Update geometry
    const double joint_rot_speed = 0.1;
    double joint_rot_t = (sin(animation_frame*joint_rot_speed) + 1.0) / 2.0;
    left_foot_joint_rot = joint_rot_t * JOINT_MIN + (1 - joint_rot_t) * JOINT_MAX;

}

void animateBody() {
    double x_speed = 0.1;
    double y_speed = 0.01;
    
    double x_rot_t = (sin(animation_frame*x_speed) + 1.0) / 2.0;
    body_x = x_rot_t * BODY_X_MIN + (1 - x_rot_t) * BODY_X_MAX;

    
    double y_rot_t = (sin(animation_frame*y_speed) + 1.0) / 2.0;
    body_y = y_rot_t * BODY_Y_MIN + (1 - y_rot_t) * BODY_Y_MAX;
    
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
        
    // Draw the whole system
    glPushMatrix();
    // Translate the whole system
    glTranslatef(body_x, body_y, 0.0);
    
    // Draw the body
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
    glTranslatef(0.0, 10, 0.0);
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
    glTranslatef(-UPPER_WIDTH, beak_y, 0.0);
    glScalef(BEAK_LENGTH, BEAK_WIDTH, 0.0);
    glColor3f(0.0, 0.0, 0.0);
    drawSquare(1.0);
    glPopMatrix();
    
    // Draw the eye
    glPushMatrix();
    glTranslatef(-UPPER_WIDTH/4, 2*FACE_HEIGHT/3, 0.0);
    glColor3f(0.0, 0.0, 0.0);
    drawColouredCircle(EYE_RADIUS);
    glPopMatrix();
    
    // Finally draw the joint
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    drawCircle(JOINT_RADIUS);
    glPopMatrix();
    
    glPopMatrix();
    /** End of head+mouth system **/
    
    /**
     ** Draw the wing
     **/
    glPushMatrix();
    // Move to the joint
    glTranslatef(0.0, BODY_HEIGHT/4, 0.0);
    // Rotate on the joint
    glRotatef(wing_joint_rot, 0.0, 0.0, 1.0);
    
    // First draw the wing
    glPushMatrix();
    // Move the center
    glTranslatef(0.0, -WING_HEIGHT/2+JOINT_OFFSET, 0.0);
    glColor3f(0.0, 0.0, 1.0);
    drawTrapzoid(wing_y);
    glPopMatrix();
    
    // Second draw the joint
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    drawCircle(JOINT_RADIUS);
    glPopMatrix();
    
    glPopMatrix();
    /** End of wing **/
    
    /**
     ** Draw the left leg+foot system
     **/
    glPushMatrix();
    
    // Translate to the joint of the leg
    glTranslatef(-BODY_BUTT_WIDTH/5-JOINT_OFFSET, -BUTT_HEIGHT-8, 0.0);
    // Rotate on the joint for the entire system
    glRotatef(left_leg_joint_rot, 0.0, 0.0, 1.0);
    
    // First draw the leg
    glPushMatrix();
    // Translate to leg's position
    glTranslatef(0.0, -LEG_HEIGHT/3, 0.0);
    // Scale the size of the leg
    glScalef(LEG_WIDTH, LEG_HEIGHT, 1.0);
    glColor3f(1.0, 0.0, 0.0);
    drawSquare(1.0);
    glPopMatrix();
    
    // Now draw the joint
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    drawCircle(JOINT_RADIUS);
    glPopMatrix();
    
    // Second draw the foot
    glPushMatrix();
    // Translate to the joint of the foot
    glTranslatef(0.0, -LEG_HEIGHT+2*JOINT_OFFSET, 0.0);
    // Foot's rotation on the foot's joint
    glRotatef(left_foot_joint_rot, 0.0, 0.0, 1.0);
    
    // Now apply foot's indiviual translation
    glPushMatrix();
    glTranslatef(-FOOT_LENGTH/2+JOINT_OFFSET, 0.0, 0.0);
    // Scale to the foot size
    glScalef(FOOT_LENGTH, FOOT_WIDTH, 1.0);
    glColor3f(0.0, 0.5, 0.5);
    drawSquare(1.0);
    glPopMatrix();
    
    // Now draw the joint
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    drawCircle(JOINT_RADIUS);
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
    glRotatef(right_leg_joint_rot, 0.0, 0.0, 1.0);
    
    // First draw the leg
    glPushMatrix();
    // Translate to leg's position
    glTranslatef(0.0, -LEG_HEIGHT/3, 0.0);
    // Scale the size of the leg
    glScalef(LEG_WIDTH, LEG_HEIGHT, 1.0);
    glColor3f(1.0, 0.0, 0.0);
    drawSquare(1.0);
    glPopMatrix();
    
    // Now draw the joint
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    drawCircle(JOINT_RADIUS);
    glPopMatrix();
    
    // Second draw the foot
    glPushMatrix();
    // Translate to the joint of the foot
    glTranslatef(0.0, -LEG_HEIGHT+2*JOINT_OFFSET, 0.0);
    // Foot's rotation on the foot's joint
    glRotatef(right_foot_joint_rot, 0.0, 0.0, 1.0);
    // Now apply foot's indiviual translation
    glPushMatrix();
    glTranslatef(-FOOT_LENGTH/2+JOINT_OFFSET, 0.0, 0.0);
    // Scale to the foot size
    glScalef(FOOT_LENGTH, FOOT_WIDTH, 1.0);
    glColor3f(0.0, 0.5, 0.5);
    drawSquare(1.0);
    glPopMatrix();
    
    // Now draw the joint
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    drawCircle(JOINT_RADIUS);
    glPopMatrix();
    
    glPopMatrix();
    
    glPopMatrix();
    /** End of right leg+foot system **/
    
    // End of the whole system
    glPopMatrix();
    
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
void drawTrapzoid(float wing_y) {
    // Scaling along y-axis, with upper line fixed
    float lower_y = -WING_HEIGHT/2 * wing_y;
    
    glBegin(GL_POLYGON);
    glVertex2f(-WING_UPPER_WIDTH/2, WING_HEIGHT/2);
    glVertex2f(WING_UPPER_WIDTH/2, WING_HEIGHT/2);
    glVertex2f(WING_LOWER_WIDTH/2, lower_y);
    glVertex2f(-WING_LOWER_WIDTH/2, lower_y);
    glEnd();
}


// Draw a circle
void drawCircle(float radius) {
    glBegin(GL_LINE_LOOP);
    int i;
    for (i = 0; i < 360; i++)
    {
        float degInRad = i * (PI/180);
        glVertex2f(cos(degInRad)*radius,sin(degInRad)*radius);
    }
    
    glEnd();
}

// Draw a circle with color
void drawColouredCircle(float radius) {
    glBegin(GL_POLYGON);
    int i;
    for (i = 0; i < 360; i++)
    {
        float degInRad = i * (PI/180);
        glVertex2f(cos(degInRad)*radius,sin(degInRad)*radius);
    }
    
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

