//
//  main.cpp
//  tut9_texture
//
//  Created by Simona Bilkova on 8/5/20.
//  Copyright © 2020 Simona Bilkova. All rights reserved.
//

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    glTranslatef(0.0, 0.0, -1.5f);
    glRotatef(camera.xRot, 0.0f, 1.0f, 0.0f);
    glRotatef(camera.yRot, 1.0f, 0.0f, 0.0f);
    glScalef(camera.position, camera.position, camera.position);
}

int n = 16;
bool wireframe = false;

float partAQuads[24] = {
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

float partATexCoord[8] = {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f
};

GLuint zBack, xRight, zFront, xLeft, bottom, top;

GLuint texture;

// load a texture from file using the stb_image library
uint32_t loadTexture(const char* filename) {
    int width, height, components;
    unsigned char* data = stbi_load(filename, &width, &height, &components, STBI_rgb);

    glPushAttrib(GL_TEXTURE_BIT);

    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
//      glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glPopAttrib();

    return id;
}

#if 0
GLuint loadTexture(const char *filename)
{
  GLuint tex = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
  if (!tex)
    return 0;

  glBindTexture(GL_TEXTURE_2D, tex);
#if 0
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
  glBindTexture(GL_TEXTURE_2D, 0);

  return tex;
}
#endif

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

float calculateY(float x) {
    float y = 0, wavelength = 1.0, amplitude = 0.25;
    float k = 2.0 * M_PI / wavelength;
    
    return y = amplitude * sin(k * x);
}

float calculateXNormal(float x) {
    return -cos(x * M_PI * 2) * 0.25 * M_PI;
}


void drawRow(float z){
    float xStart = -1, zStart =1, xStep = 2.0f / (float)n, zStep = 2.0f / (float)n, y, x, nextY, j;
    j = z;
    z = - (z * zStep);
    float actualZ = zStart + z;
    for (int i = 0; i < n; i++) {
        x = xStart + i * xStep;
        y = calculateY(x);
        nextY = calculateY(x + xStep);
        
        glNormal3f(calculateXNormal(x), 1.0  , 0);
        glTexCoord2f(0 + 1.0f / n * i, 0 + 1.0f / n * (j + 1));
        glVertex3f(x        , y    , actualZ - zStep);
        
        glTexCoord2f(0 + 1.0f / n * i, 0 + 1.0f / n * j);
        glVertex3f(x,         y    , actualZ);
        
        glNormal3f(calculateXNormal(x + xStep), 1.0, 0.0);
        glTexCoord2f(0 + 1.0f / n * (i + 1), 0 + 1.0f / n * (j + 1));
        glVertex3f(x + xStep, nextY, actualZ - zStep);
        
        glTexCoord2f(0 + 1.0f / n * (i + 1), 0+ 1.0f / n * j);
        glVertex3f(x + xStep, nextY, actualZ);
        
#if 0
        glNormal3f(x ,        y    , actualZ);
        glVertex3f(x,         y    , actualZ);
        glNormal3f(x + xStep, nextY, actualZ - zStep);
        glVertex3f(x + xStep, nextY, actualZ - zStep);
        glNormal3f(x + xStep, nextY, actualZ);
        glVertex3f(x + xStep, nextY, actualZ);
#endif
    }
}

void drawBox(){
    glBindTexture(GL_TEXTURE_2D, zBack);
    
    glBegin(GL_QUAD_STRIP);
    int x = 0, y = 1, z = 2;
    for (int i = 0; i < 24; i = i + 3) {
        glVertex3f(partAQuads[i + x], partAQuads[i + y], partAQuads[i + z]);
        
    }
#if 0
    glNormal3f(0.0, 0.0, 1.0);
    glTexCoord2f(0, 1);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    
    glTexCoord2f(1, 1);
    glVertex3f(1.0f, -1.0f, -1.0f);
    
    glTexCoord2f(1, 0);
    glVertex3f(1.0f, 1.0f, -1.0f);
    
    
    glTexCoord2f(0, 0);
    glVertex3f(-1.0f, 1.0f, -1.0f);
#endif
    
    glEnd();
}

void drawGrid(){
    
    glBindTexture(GL_TEXTURE_2D, texture);
        for (int i = 0; i < n; i++) {
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
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);

    GLfloat light[] = {1, 1, 1, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, light);
    
    GLfloat light2[] = {-1, 1, 0, 0};
    glLightfv(GL_LIGHT1, GL_POSITION, light2);
    
    GLfloat light3[] = {0, 0, -1.0, 0};
    glLightfv(GL_LIGHT1, GL_POSITION, light3);
    
    setCamera();
    
    if (wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    drawAxes(10.0);
    drawBox();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    drawGrid();
    
#if 0
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(-1, -1);
    glTexCoord2f(1, 0);
    glVertex2f(1, -1);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);
    glTexCoord2f(0, 1);
    glVertex2f(-1, 1);
    glEnd();
#endif

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
            wireframe = !wireframe;
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
    glutCreateWindow("Texture viewer");
    
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseEvent);
    glutMotionFunc(mouseMotion);
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutIdleFunc(update);
    
    if (argc < 2) {
        printf("Specify an image filename to display.\n");
        return EXIT_FAILURE;
    }
    
    const char* file1 = "zpos.png";
    
    zBack = loadTexture(file1);
    texture = loadTexture(argv[1]);
    if (!texture || !zBack)  {
        printf("No texture created; exiting.\n");
        return EXIT_FAILURE;
    }
    
    
    glutMainLoop();
}


