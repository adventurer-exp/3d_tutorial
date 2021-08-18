//
//  main.cpp
//  tut10_cannon
//
//  Created by Simona Bilkova on 15/5/20.
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

typedef struct { float x, y, z; } vec3f;
typedef struct { float r, aplha, phi; } vec3fPolar;
vec3fPolar canon = {
    0.5,
    45.0,
    0.0
};

typedef struct { vec3f r0, v0, r, v; } state;
state projectile = {
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 }
};

bool go = false;
float coord = 0.0f, platform_phi = 0.0f, platform_alpha = 0.0f;

const float ADJUST_ZOOM = 100.0f;
bool rotateCamera = false, zoom = false;


float degToRad(float deg) {
    return deg * M_PI / 180;
}

float radToDeg(float rad) {
    return rad * 180 / M_PI;
}

void setCamera() {
    glTranslatef(0.0, 0.0, -1.5f);
    glRotatef(camera.xRot, 0.0f, 1.0f, 0.0f);
    glRotatef(camera.yRot, 1.0f, 0.0f, 0.0f);
    glScalef(camera.position, camera.position, camera.position);
}

void convertPolToCar() {
    projectile.r0.x = canon.r * cosf(degToRad(canon.aplha + platform_alpha)) * cosf(degToRad(canon.phi + platform_phi));
    printf("r x: %.2f\n", projectile.r0.x);
    
    projectile.r0.z = canon.r * cosf(degToRad(canon.aplha + platform_alpha)) * -sinf(degToRad(canon.phi+ platform_phi));
    printf("r y: %.2f\n", projectile.r0.y);
    
    projectile.r0.y = canon.r * sin(degToRad(canon.aplha + platform_alpha));
    printf("r z: %.2f\n", projectile.r0.z);
    
    float speed = 1.2f;
    projectile.v0.x = canon.r * speed * cosf(degToRad(canon.aplha + platform_alpha)) * cosf(degToRad(canon.phi + platform_phi));
    printf("v x: %.2f\n", projectile.v0.x);
    projectile.v0.z = canon.r * speed * cosf(degToRad(canon.aplha + platform_alpha)) * -sinf(degToRad(canon.phi+ platform_phi));
    printf("v y: %.2f\n", projectile.v0.y);
    projectile.v0.y = canon.r * speed * sin(degToRad(canon.aplha + platform_alpha));
    printf("v z: %.2f\n", projectile.v0.z);
}

void drawProjectile(){
    if (go) {
        glColor3f(1.0, 1.0, 1.0);
            glPushMatrix();

            glTranslatef(projectile.r.x, projectile.r.y, projectile.r.z);

        glutWireSphere(0.05, 8, 8);

        glPopMatrix();
    } else {
        glColor3f(1.0, 1.0, 1.0);
            glPushMatrix();

            glTranslatef(projectile.r0.x, projectile.r0.y, projectile.r0.z);
        
        glutWireSphere(0.05, 8, 8);

        glPopMatrix();
    }
    
    
}

float calculateProjMotion(float dt, float v0y, float ry){
    return 1.0 / 2.0 * -0.25 * dt * dt + v0y * dt + ry;
}

void drawTrajectoryNumerical()
{
    float current_time = glutGet(GLUT_ELAPSED_TIME);
    current_time /= 5000.0;
    
    float left = projectile.r.x;
    float front = projectile.r.z;
    float dt = .5;
    float multiple_dt = 0.0;
    
    float x = projectile.r.x;
    float y = projectile.r.y;
    float z = projectile.r.z;
    
    glPushMatrix();
    
    glBegin(GL_LINE_STRIP);
    glColor3f(1, 0.862745, 0.294117);
    
    while (y >= 0.0)
    {
        
        
            x = left + projectile.v0.x * multiple_dt;
            z = front + projectile.v0.z * multiple_dt;
            y = calculateProjMotion(multiple_dt, projectile.v0.y, projectile.r.y);
            
            x = x - projectile.r.x + projectile.r0.x;
            y = y - projectile.r.y + projectile.r0.y;
            z = z - projectile.r.z + projectile.r0.z;
        printf("x = %f, y = %f, z = %f \n", x, y, z);
            glVertex3f(x, y, z);
        multiple_dt += dt;
    }
    
    glEnd();
    glPopMatrix();
}

void updateProjectileStateNumerical(float dt)
{
    if (projectile.r.y < 0.0)
        go = false;
    // Euler integration
    
    // Position
    projectile.r.x += projectile.v.x * dt;
    projectile.r.y += projectile.v.y * dt;
    projectile.r.z += projectile.v.z * dt;
     
    // Velocity
    projectile.v.y += -0.25 * dt;
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
    
    static float lastT = -1.0;
    float t, dt;
       
       
    t = glutGet(GLUT_ELAPSED_TIME);
    t /= 600.0;
       
    if (lastT < 0.0) {
        lastT = t;
        return;
    }
    dt = t - lastT;
    
    lastT = t;
    
    convertPolToCar();
    
    if (go)
        updateProjectileStateNumerical(dt);

    
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

void drawPlatform() {
    if (coord == 0.0f)
        coord = sinf(M_PI / (float)4 );
    
    drawAxes(0.5);
    glBegin(GL_LINE_LOOP);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glVertex3f(coord, 0.0f, -coord);
    glVertex3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-coord, 0.0f, -coord);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-coord, 0.0f, coord);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glVertex3f(coord, 0.0f, coord);
    
    glEnd();
}

void drawTurret(){
    glPushMatrix();
    drawAxes(0.2);
    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0, -0.05, 0.0);
    glVertex3f(0.0, 0.05, 0.0);
    glVertex3f(canon.r, 0.05, 0.0);
    glVertex3f(canon.r, -0.05, 0.0);
    glEnd();
    glPopMatrix();
}


void display(void)
{
    GLenum err;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setCamera();
    drawAxes(1.0);
    glPushMatrix();
    glRotatef(platform_alpha, 0.0, 0.0, 1.0);
    glRotatef(platform_phi, 0.0, 1.0, 0.0);
//    glRotatef(20, 0.0f, 0.0, 1.0);
    drawPlatform();
    glPushMatrix();
    glRotatef(canon.phi, 0.0, 1.0, 0.0);
    glRotatef(canon.aplha, 0.0, 0.0, 1.0);
    
//    glRotatef(30, 0.0, 1.0, 0.0);
    
    drawTurret();
    glPopMatrix();
    glPopMatrix();
    drawProjectile();
    
    drawTrajectoryNumerical();
    
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
            exit(EXIT_SUCCESS);
            break;
        case 'A':
            canon.aplha += 1;
	            break;
        case 'a':
            canon.aplha -= 1;
            break;
        case 'Q':
            canon.phi += 1;
            break;
        case 'q':
            canon.phi -= 1;
            break;
        case 'Z':
            platform_phi += 1;
            break;
        case 'z':
            platform_phi -= 1;
            break;
        case 'S':
            platform_alpha += 1;
            break;
        case 's':
            platform_alpha -= 1;
            break;
        case ' ':
            if (!go) {
                go = true;
                projectile.v.x = projectile.v0.x;
                projectile.v.y = projectile.v0.y;
                projectile.v.z = projectile.v0.z;
                
                projectile.r.x = projectile.r0.x;
                projectile.r.y = projectile.r0.y;
                projectile.r.z = projectile.r0.z;
            }
            break;
        default:
            break;
    }
    convertPolToCar();
    
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


