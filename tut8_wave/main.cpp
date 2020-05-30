//
//  main.cpp
//  tut7_grid
//
//  Created by Simona Bilkova on 24/4/20.
//  Copyright © 2020 Simona Bilkova. All rights reserved.
//

#define GL_SILENCE_DEPRECATION

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>


typedef struct {
    float xRot, yRot, lastX, lastY, position, lastPosition;
} camera_t;

typedef struct {
    bool wiretframe, normals, tangent, binormal;
} global_t;

/*
 a = amplitude
 k =
 w = wavelength
 */
typedef struct  {
    float a, k , w;
} sinewave;

sinewave sw = {
    0.25, 2 * M_PI, 0.25 * M_PI
};

global_t global = {
    true, false, false, false
};

camera_t camera = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};

camera_t data = {
    0.0f, 0.0f, -1.0f, -1.0f, 0.0f, -1.0f
};

const float ADJUST_ZOOM = 100.0f;
bool rotateCamera = false, zoom = false, animation = false;
int n = 16;
float animationX;
static float t_last = 0.0;


void setCamera() {
    glTranslatef(0.0, 0.0, -5.0f);
    glRotatef(camera.xRot, 0.0f, 1.0f, 0.0f);
    glRotatef(camera.yRot, 1.0f, 0.0f, 0.0f);
    glScalef(camera.position, camera.position, camera.position);
}

// Idle callback for animation
void update(void)
{
    float t, dt;
    /* Get elapsed time and convert to s */
    t = glutGet(GLUT_ELAPSED_TIME);
    t /= 5000.0;
    
    if (rotateCamera){
        camera.xRot = camera.lastX + data.xRot;
        camera.lastX = camera.xRot;
        
        camera.yRot = camera.lastY + data.yRot;
        camera.lastY = camera.yRot;
    }
    
    if (zoom) {
        camera.position = camera.lastPosition + data.position;
        camera.lastPosition = camera.position;
    }
    
    //    Chnage in time
    dt = t - t_last;
    
    /* Update velocity and position */
    if (animation)
        animationX += dt;
    
    t_last = t;

    
    glutPostRedisplay();
}

void drawAxes(float l)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(l, 0.0, 0.0);
    glColor3f(0, 1, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, l, 0.0);
    glColor3f(0, 0, 1);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, l);
    glEnd();
    glColor3f(1, 1, 1);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void drawSquare(float r, float g, float b, float dimensions)
{
    float coord = dimensions / 2.0;
    glBegin(GL_TRIANGLES);
    glColor3f(r, g, b);
    glNormal3f(-coord, 0.0, coord);
    glVertex3f(-coord, 0.0, coord);
    glVertex3f(coord, 0.0, coord);
    glVertex3f(coord, 0.0, -coord);
    
    glVertex3f(-coord, 0.0, coord);
    glVertex3f(coord, 0.0, -coord);
    glVertex3f(-coord, 0.0, -coord);
    glEnd();
}

void drawVector(float x, float y, float z, float a, float b, float s, bool normalize, float red, float green, float blue) {
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(red, green, blue);

//    float length = sqrt(a * a + b * b);
    glVertex3f(x, y, z);
    if (normalize) {
        float magnitude = sqrt(a * a + b * b);
        glVertex3f(x + a / magnitude * s, y + b / magnitude * s, z);
    } else {
        glVertex3f(x + a * s, y + b * s, z);
    }
    
    glColor3f(1, 1, 1);
    glEnd();
    glEnable(GL_LIGHTING);

    
}

float calculateY(float x) {
    return sw.a * sin(sw.k * x);
}

// dvs = is Derivate to be calculated
void calculateSinWave(sinewave sw, float x, float t, float *y, bool dvs, float *dvdx) {
    
}


// dvs = is Derivate to be calculated
void calculateSinWaveSum(sinewave sw, float x, float t, float *y, bool dvs, float *dvdx) {
    
}



void drawRow(float z){
    float xStart = -1, zStart =1, xStep = 2.0f / (float)n, zStep = 2.0f / (float)n, y, x, nextY;
    z = - (z * zStep);
    float actualZ = zStart + z;
    for (int i = 0; i < n; i++) {
        x = xStart + i * xStep;
        y = calculateY(x + animationX);
        nextY = calculateY(x + xStep + animationX);
        
        glNormal3f(-cos(x * M_PI * 2) * 0.25 * M_PI, 1.0, 0.0);
        glVertex3f(x,         y    , actualZ);
        glVertex3f(x        , y    , actualZ - zStep);
    }
}

void drawNormals(float length){
    glColor3f(1.0, 1.0, 0.0);
    float xStart = -1, zStart =1, xStep = 2.0f / (float)n, zStep = 2.0f / (float)n, y, x, nextY;
    
    for (int j = 0; j < n; j++) {
        float z = -(j * zStep);
    
    for (int i = 0; i < n; i++) {
        x = xStart + i * xStep;
        float actualZ = zStart + z;
        y = calculateY(x);
        nextY = calculateY(x + xStep);
        
        if (global.normals)
            drawVector(x, y, actualZ, -cos(x * sw.k) * sw.w, 1.0, 0.1, false, 0, 1, 0);
        
        if (global.tangent)
            drawVector(x, y, actualZ, 1.0, cos(x * sw.k) * sw.w, 0.1, true, 1, 0, 0);
        
#if 0
        drawVector(x, y, actualZ, 1.0, cos(0.25 * x * M_PI) * M_PI, 0.1, true, 1, 0, 0);
        
        glBegin(GL_LINES);
        
        glVertex3f(x,         y,              actualZ);
        glVertex3f(x,         y + length,     actualZ);
        
        glVertex3f(x + xStep, nextY,          actualZ);
        glVertex3f(x + xStep, nextY + length, actualZ);
        
        glVertex3f(x + xStep, nextY,          actualZ - zStep);
        glVertex3f(x + xStep, nextY + length, actualZ - zStep);
        
        glVertex3f(x,         y,              actualZ - zStep);
        glVertex3f(x,         y,              actualZ - zStep);
        glEnd();
#endif
    }
    }
}

void drawGrid(){
        for (int i = 0; i < n   ; i++) {
            glBegin(GL_QUAD_STRIP);
            drawRow(i);
            glEnd();
        }
    
}



void display(void)
{
    GLenum err;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light[] = { 0.0, 1.0, 0.0, 0.0};
    GLfloat cyan[] = { 0.0, 1, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, cyan);
    glLightfv(GL_LIGHT0, GL_POSITION, light);
    

    //    glMaterialf(GL_FRONT_AND_BACK, GL_SPECULAR, 100);
    
    setCamera();
    if (global.wiretframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    drawAxes(2.0);
    
    drawGrid();
    drawNormals(0.1);
        
    glutSwapBuffers();
    
    // Check for errors
    while ((err = glGetError()) != GL_NO_ERROR)
        printf("%s\n",gluErrorString(err));
}

void stopRotation() {
    data.xRot = 0.0f;
    data.yRot = 0.0f;
    rotateCamera = false;
}

void stopZoom() {
    data.lastPosition = -1.0f;
    zoom = false;
}

void mouseEvent(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        data.lastX = -1.0f;
        data.lastY = -1.0f;
        rotateCamera = true;
        stopZoom();
    }
    
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP){
        stopRotation();
    }
    
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
        data.lastPosition = -1.0f;
        zoom = true;
        stopRotation();
    }
    
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP){
        stopZoom();
    }
    
    glutPostRedisplay();
}

void mouseMotion(int x, int y) {
    if (rotateCamera) {
        if (data.lastX == -1.0f)
            data.lastX = x;
        data.xRot = x - data.lastX;
        data.lastX = x;
        
        if (data.lastY == -1.0f)
            data.lastY = y;
        data.yRot = y - data.lastY;
        data.lastY = y;
    }
    
    if (zoom){
        if (data.lastPosition == -1.0f)
            data.lastPosition = y;
        data.position = y - data.lastPosition;
        data.position /= ADJUST_ZOOM;
        data.lastPosition = y;
    }
    
    glutPostRedisplay();
}

void keyboardCB(unsigned char key, int x, int y)
{
    switch (key) {
        case 27:
        case 'q':
            exit(EXIT_SUCCESS);
            break;
        case 'p':
            global.wiretframe = !global.wiretframe;
            break;
        case 'n':
            global.normals = !global.normals;
            break;
        case 't':
            global.tangent = !global.tangent;
            break;
        case 'b':
            global.binormal = !global.binormal;
            break;
        case 'h':
            animation = !animation;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void myReshape(int w, int h)
{
    glViewport(0, 0, w *2, h *2);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75, (float)w / (float)h, 0.01, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/*  Main Loop
 *  Open window with initial window size, title bar,
 *  RGBA display mode, and handle input events.
 */
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(500, 500);
    glutCreateWindow("Teapot");
    
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseEvent);
    glutMotionFunc(mouseMotion);
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutIdleFunc(update);
    
    glutMainLoop();
}


