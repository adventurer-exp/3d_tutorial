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

camera_t camera = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};

camera_t data = {
    0.0f, 0.0f, -1.0f, -1.0f, 0.0f, -1.0f
};

const float ADJUST_ZOOM = 100.0f;
bool rotateCamera = false, zoom = false;

void setCamera() {
    glTranslatef(0.0, 0.0, -15.0f);
    glRotatef(camera.xRot, 0.0f, 1.0f, 0.0f);
    glRotatef(camera.yRot, 1.0f, 0.0f, 0.0f);
    glScalef(camera.position, camera.position, camera.position);
}

// Idle callback for animation
void update(void)
{
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
    
    
    glutPostRedisplay();
}

void drawAxes(float l)
{
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

void drawRow(float z){
    float xStart = -10, zStart =10;
    for (int i = 0; i < 10; i++) {
        float x = xStart + i * 2;
        float actualZ = zStart + z;
        glBegin(GL_TRIANGLES);
        glVertex3f(x,     0.0, actualZ);
        glVertex3f(x + 2, 0.0, actualZ);
        glVertex3f(x + 2, 0.0, actualZ - 2);
        
        glVertex3f(x,     0.0, actualZ);
        glVertex3f(x + 2, 0.0, actualZ - 2);
        glVertex3f(x    , 0.0, actualZ - 2);
        glEnd();
    }
}

void drawNormals(){
    glColor3f(1.0, 1.0, 0.0);
    float xStart = -10, zStart =10;
    
    for (int j = 0; j < 10; j++) {
        float z = -(j * 2);
    
    for (int i = 0; i < 10; i++) {
        float x = xStart + i * 2;
        float actualZ = zStart + z;
        glBegin(GL_LINES);
        
        glVertex3f(x,     0.0, actualZ);
        glVertex3f(x,     1.0, actualZ);
        
        glVertex3f(x + 2, 0.0, actualZ);
        glVertex3f(x + 2, 1.0, actualZ);
        
        glVertex3f(x + 2, 0.0, actualZ - 2);
        glVertex3f(x + 2, 1.0, actualZ - 2);
        
        glVertex3f(x    , 0.0, actualZ - 2);
        glVertex3f(x    , 0.0, actualZ - 2);
        glEnd();
    }
    }
}

void drawGrid(){
        for (int i = 0; i < 10; i++) {
            drawRow(-(i * 2));
        }
    
}



void display(void)
{
    GLenum err;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setCamera();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    drawAxes(10.0);
    drawGrid();
    drawNormals();
    
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


