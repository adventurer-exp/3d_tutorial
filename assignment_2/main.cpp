//
//  main.cpp
//  tut7_grid
//
//  Created by Simona Bilkova on 24/4/20.
//  Copyright © 2020 Simona Bilkova. All rights reserved.
//

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <array>
#include <vector>
#include <stdbool.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

typedef struct {
    float xRot, yRot, lastX, lastY, position, lastPosition;
} camera_t;

typedef struct { float r, aplha, phi; } vec3fPolar;

typedef struct {
    float x, y, z;
} vec3f;

typedef struct { vec3f r0, v0, r, v; } state;

typedef struct {
    bool wiretframe, normals, rotateCamera, zoom, animation, allowed_to_fire, textures, lighting, finished;
    int cannon_angle, turret_angle, island_health, points;
    float animationX, global_dt, speed, last_spawn_boat_t, last_fired;
} global_t;

typedef struct {
    float x_start, z_start;
    int n;
} grid_t;

typedef struct  {
    float a, k , w;
} sinewave;

grid_t space = { 20, 20, 100 };
const int MAX_N = 101;

typedef struct {
    vec3f position;
    float rotation_y, rotation_z, centre_rot = 0.0, last_fired = 0.0;
    state projectile;
} boat_t;

boat_t* boat = new boat_t {
    { 0.0, 0.0, 0.0 },
    0.0
};

//Used at starting point for island's projectile
state* projectile = new state {
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0 }
};

vec3fPolar canon = {
    0.15,
    45.0,
    0.0
};

sinewave sw1 = { 1.5, 0.125 * M_PI, .4 * M_PI};
sinewave sw2 = { 0.75, 0.4 * M_PI, 0.35 * M_PI};
sinewave sw3 = { 1.75, .25 * M_PI, 1.75 * M_PI};
sinewave sw4 = { .75, 0.05 * M_PI, 0.25 * M_PI};

global_t global = {
    true, false, true, false, false, true, true, true, false,
    30, 0, 100, 0,
    0.0, 0.10, 7.0, 0.0, 0.0
};

camera_t camera = {
    3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};

camera_t data = {
    0.0f, 0.0f, -1.0f, -1.0f, 0.0f, -1.0f
};

std::array<vec3f, 24> island_base;
std::array<vec3f, 24> island_normals;

std::vector<state*> boats_proj;
std::vector<state*> island_proj;

std::vector<boat_t*> boats;

vec3f base_top[12];
vec3f base_top_normals[12];

// Used for textures
GLuint zPos, xPos, zNeg, xNeg, yNeg, yPos, ground;

GLUquadricObj *base = gluNewQuadric();
GLUquadricObj *turret = gluNewQuadric();
GLUquadricObj *cannon = gluNewQuadric();

const float ADJUST_ZOOM = 100.0f;
static float t_last = 0.0;

vec3f pos = { 0.0, 0.0, 0.0 };
vec3f normal = { 0.0, 0.0, 0.0 };

vec3f grid [MAX_N][MAX_N];
vec3f normals [MAX_N][MAX_N];
vec3f tangents [MAX_N][MAX_N];

float degToRad(float deg) {
    return deg * M_PI / 180;
}

float radToDeg(float rad) {
    return rad * 180 / M_PI;
}

float calculateProjMotion(float dt, float v0y, float ry){
    return 1.0 / 2.0 * -0.25 * dt * dt + v0y * dt + ry;
}

float calcualteDistanceFromCentre(boat_t* boat) {
    float x = boat->position.x, y = boat->position.y, z = boat->position.z;
    float d = sqrtf((x)*(x) + (y)*(y) + (z)*(z));
    return d;
}

float calcualteDistanceFromCentre(state* projectile) {
    float x = projectile->r.x, y = projectile->r.y, z = projectile->r.z;
    float d = sqrtf((x)*(x) + (y)*(y) + (z)*(z));
    return d;
}

float calculateDistance(state* projectile, boat_t* boat) {
    float x = projectile->r.x, y = projectile->r.y, z = projectile->r.z, x1 = boat->position.x, y1 = boat->position.y, z1 = boat->position.z;
    float d = sqrtf((x - x1)*(x - x1) + (y - y1)*(y - y1) + (z - z1)*(z - z1));
    return d;
}

float calculate_dx(sinewave x_wave, sinewave z_wave, float x, float z) {
    return x_wave.k * x_wave.a * cosf(x_wave.k * x +  x_wave.w * global.animationX) * cosf(z_wave.k * z + z_wave.w * global.animationX);
}

float calculate_dz(sinewave x_wave, sinewave z_wave, float x, float z) {
    return z_wave.k * x_wave.a * sinf(z_wave.k * z + z_wave.w * global.animationX) * -sinf(x_wave.k * x +  x_wave.w * global.animationX);
}

void drawTrajectoryNumerical(state& projectile)
{
    float current_time = glutGet(GLUT_ELAPSED_TIME);
    current_time /= 600.0;
    
    float left = projectile.r.x;
    float front = projectile.r.z;
    float multiple_dt = 0.0;
    
    float x = projectile.r.x;
    float y = projectile.r.y;
    float z = projectile.r.z;
    
    glPushMatrix();
    glBegin(GL_LINE_STRIP);
    glColor3f(1, 0.862745, 0.294117);
    
    while (y >= -2.5)
    {
        x = left + projectile.v0.x * multiple_dt;
        z = front + projectile.v0.z * multiple_dt;
        y = calculateProjMotion(multiple_dt, projectile.v0.y, projectile.r.y);
        
        x = x - projectile.r.x + projectile.r0.x;
        y = y - projectile.r.y + projectile.r0.y;
        z = z - projectile.r.z + projectile.r0.z;
        glVertex3f(x, y, z);
        multiple_dt += global.global_dt;
    }
    
    glEnd();
    glPopMatrix();
}

void initBoat(boat_t* boat){
    float x, z;
    int r = rand() % 4;
    if (r == 0) {
        x = space.x_start;
        z = rand() % (int)(space.z_start * 2 + 1) - space.x_start;
    } else if (r == 1) {
        x = rand() % (int)(space.z_start * 2 + 1) - space.x_start;
        z = space.z_start;
    } else if (r == 2) {
        x = -space.x_start;
        z = rand() % (int)(space.z_start * 2 + 1) - space.x_start;
    } else {
        x = rand() % (int)(space.z_start * 2 + 1) - space.x_start;
        z = -space.z_start;
    }
    boat->position.x = x;
    boat->position.z = z;
}

void convertPolToCar(state& projectile) {
    projectile.r0.x = canon.r * cosf(degToRad(global.cannon_angle)) * cosf(degToRad(global.turret_angle + 90.0)) ;
    projectile.r0.z = canon.r * cosf(degToRad(global.cannon_angle)) * -sinf(degToRad(global.turret_angle + 90.0));
    projectile.r0.y = canon.r * sinf(degToRad(global.cannon_angle));
    
    projectile.v0.x = canon.r * global.speed * cosf(degToRad(global.cannon_angle)) * cosf(degToRad(global.turret_angle + 90.0));
    projectile.v0.z = canon.r * global.speed * cosf(degToRad(global.cannon_angle)) * -sinf(degToRad(global.turret_angle + 90.0));
    projectile.v0.y = canon.r * global.speed * sinf(degToRad(global.cannon_angle));
}

void updateProjectileStateNumerical(state* projectile, float dt)
{
    dt *= 10;
    
    projectile->r.x += projectile->v.x * dt;
    projectile->r.y += projectile->v.y * dt;
    projectile->r.z += projectile->v.z * dt;
    
    projectile->v.y += -0.25 * dt;
}

float calculateY(sinewave sw_x, sinewave sw_z, float x, float z) {
    return sw_x.a * sinf(sw_x.k * x +  sw_x.w * global.animationX) * cosf(sw_z.k * z + sw_z.w * global.animationX);
}

void updateBoat(boat_t* boat, float dt){
    float y = calculateY(sw1, sw3, boat->position.x, boat->position.z);
    y += calculateY(sw2, sw4, boat->position.x, boat->position.z);
    boat->position.y = y;
    
    float r = 3;
    if (calcualteDistanceFromCentre(boat) >  r + 1 && boat->centre_rot == 0.0) {
        dt *= 2;
        if (boat->position.x < 0){
            boat->position.x += dt;
        } else {
            boat->position.x -= dt;
        }
        
        if (boat->position.z < 0){
            boat->position.z += dt;
        } else {
            boat->position.z -= dt;
        }
    } else {
        if (boat->centre_rot == 0.0){
            boat->centre_rot = asin(boat->position.z / (r + 1.0));
        }
        boat->centre_rot += dt;
        boat->position.z = sinf(boat->centre_rot) * 4.0;
        boat->position.x = cosf(boat->centre_rot) * 4.0;
    }
    
    float dx = calculate_dx(sw1, sw3, boat->position.x, boat->position.z);
    dx += calculate_dx(sw2, sw4, boat->position.x, boat->position.z);
    float dz = calculate_dz(sw1, sw3, boat->position.x, boat->position.z);
    dz += calculate_dz(sw2, sw4, boat->position.x, boat->position.z);
    
    boat->rotation_z = radToDeg(atanf(1.0/-dx));
    
    boat->rotation_y = radToDeg(atanf(1.0/-dz));
    
    boat->projectile.r0.x = boat->position.x;
    boat->projectile.r0.z = boat->position.z;
    boat->projectile.r0.y = boat->position.y;
    
    float len = calcualteDistanceFromCentre(boat);
    
    boat->projectile.v0.x = -boat->position.x / len;
    boat->projectile.v0.z = -boat->position.z / len;
    boat->projectile.v0.y = 0.5 * 0.25 * calcualteDistanceFromCentre(boat) - boat->position.y / calcualteDistanceFromCentre(boat);
    
}

bool checkCollision(state& projectile) {
    float l = calcualteDistanceFromCentre(&projectile);
    if (l <= 3)
        return true;
    return false;
}

bool checkIslandProjCollision(state& projectile) {
    int size = (int)boats.size();
    int toDelete = -1;
    bool deleted = false;
    for (int i = 0; i < size; i++) {
        if (calculateDistance(&projectile, boats[i]) < 2) {
            toDelete = i;
        }
    }
    if (toDelete != -1) {
        boat_t* boat;
        boat = boats[toDelete];
        boats.erase(boats.begin() + toDelete);
        delete boat;
        deleted = true;
    }
    
    return deleted;
}

void initProjectile(state& to, state from){
    to.r0.x = from.r0.x;
    to.r0.y = from.r0.y;
    to.r0.z = from.r0.z;
    
    to.v0.x = from.v0.x;
    to.v0.y = from.v0.y;
    to.v0.z = from.v0.z;
    
    to.r.x = from.r0.x;
    to.r.y = from.r0.y;
    to.r.z = from.r0.z;
    
    to.v.x = from.v0.x;
    to.v.y = from.v0.y;
    to.v.z = from.v0.z;
}

void drawProjectile(state& projectile, bool boat_p){
    glColor3f(1.0, 1.0, 1.0);
    glPushMatrix();
    glTranslatef(projectile.r.x, projectile.r.y, projectile.r.z);
    glutSolidSphere(0.1, 4, 4);
    glPopMatrix();
}

void drawSkyBox(){
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, xNeg);
    glBegin(GL_QUADS);
    float p = 20;
    glTexCoord2f(0, 0);
    glVertex3f(-p - 1, p + 1, -p - 1);
    glTexCoord2f(1, 0);
    glVertex3f(-p - 1, p + 1, p + 1);
    glTexCoord2f(1, 1);
    glVertex3f(-p - 1, -p - 1, p + 1);
    glTexCoord2f(0, 1);
    glVertex3f(-p - 1, -p - 1, -p - 1);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, zNeg);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(p + 1, p + 1, -p - 1);
    glTexCoord2f(1, 0);
    glVertex3f(-p - 1, p + 1, -p - 1);
    glTexCoord2f(1, 1);
    glVertex3f(-p - 1, -p - 1, -p - 1);
    glTexCoord2f(0, 1);
    glVertex3f(p + 1, -p - 1, -p - 1);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, xPos);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(p + 1, p + 1, p + 1);
    glTexCoord2f(1, 0);
    glVertex3f(p + 1, p + 1, -p - 1);
    glTexCoord2f(1, 1);
    glVertex3f(p + 1, -p - 1, -p - 1);
    glTexCoord2f(0, 1);
    glVertex3f(p + 1, -p - 1, p + 1);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, zPos);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(-p - 1, p + 1, p + 1);
    glTexCoord2f(1, 0);
    glVertex3f(p + 1, p + 1, p + 1);
    glTexCoord2f(1, 1);
    glVertex3f(p + 1, -p - 1, p + 1);
    glTexCoord2f(0, 1);
    glVertex3f(-p - 1, -p - 1, p + 1);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, yPos);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 1);
    glVertex3f(p + 1, p + 1, p + 1);
    glTexCoord2f(0, 1);
    glVertex3f(-p - 1, p + 1, p + 1);
    glTexCoord2f(0, 0);
    glVertex3f(-p - 1, p + 1, -p - 1);
    glTexCoord2f(1, 0);
    glVertex3f(p + 1, p + 1, -p - 1);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, yNeg);
    glBegin(GL_QUADS);
    glTexCoord2f(1, 1);
    glVertex3f(p + 1, -p - 1, -p - 1);
    glTexCoord2f(0, 1);
    glVertex3f(-p - 1, -p - 1, -p - 1);
    glTexCoord2f(0, 0);
    glVertex3f(-p - 1, -p - 1, p + 1);
    glTexCoord2f(1, 0);
    glVertex3f(p + 1, -p - 1, p + 1);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, 0);
    if (global.lighting)
        glEnable(GL_LIGHTING);
    
}

void setCamera() {
    glTranslatef(0.0, -2.5, -5.0f);
    glRotatef(camera.xRot, 1.0f, 0.0f, 0.0f);
    glRotatef(camera.yRot, 0.0f, 1.0f, 0.0f);
    
    drawSkyBox();
    glScalef(camera.position, camera.position, camera.position);
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
    if (global.lighting)
        glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}


void initIsland() {
    float r = 3, y_bottom = -0.5, y_top = 1.5;
    int parts = 12;
    float angle = (2.0 * M_PI) / parts;
    
    int j = 0;
    
    float y_norm =  sqrt(3) * sinf(M_PI / 4.0);
    for (int i = 0; i < parts; i++) {
        float x = r * cosf(angle * i);
        float z = r * sinf(angle * i);
        float add_z = 2.0 * z, add_x = x * 2.0;
        
        pos = { x , y_bottom, z };
        island_base[j] = pos;
        
        normal = { add_x , y_bottom - y_norm,  add_z};
        island_normals[j] = normal;
        j++;
        
        pos = { x , y_top,  z };
        island_base[j] = pos;
        
        normal = { add_x , y_top + y_norm, add_z};
        island_normals[j] = normal;
        j++;
    }
}

void drawRectangle(float x, float z, float normal_x, float normal_y, float normal_z) {
    glPushMatrix();
    glBegin(GL_QUADS);
    glNormal3f(normal_x, normal_y, normal_z);
    float half_x = x / 2.0, half_z = z / 2.0;
    glVertex3f(-half_x, -0.125, half_z);
    glVertex3f(half_x, -0.125, half_z);
    glVertex3f(half_x, 0.125, half_z);
    glVertex3f(-half_x, 0.125, half_z);
    glEnd();
    glPopMatrix();
}

void drawBoat(){
    drawRectangle(0.5, 0.25, 0, 0, 1);
    
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glVertex3f(0.25, -0.125, 0.125);
    glVertex3f(0.25, -0.125, -0.125);
    glVertex3f(0.25, 0.125, -0.125);
    glVertex3f(0.25, 0.125, 0.125);
    
    glNormal3f(0, 0, -1);
    glVertex3f(0.25, -0.125, -0.125);
    glVertex3f(-0.25, -0.125, -0.125);
    glVertex3f(-0.25, 0.125, -0.125);
    glVertex3f(0.25, 0.125, -0.125);
    
    glNormal3f(-1, 0, 0);
    glVertex3f(-0.25, -0.125, -0.125);
    glVertex3f(-0.25, -0.125, 0.125);
    glVertex3f(-0.25, 0.125, 0.125);
    glVertex3f(-0.25, 0.125, -0.125);
    
    glNormal3f(0, 1, 0);
    glVertex3f(-0.25, 0.125, 0.125);
    glVertex3f(0.25, 0.125, 0.125);
    glVertex3f(0.25, 0.125, -0.125);
    glVertex3f(-0.25, 0.125, -0.125);
    
    glNormal3f(0, -1, 0);
    glVertex3f(-0.25, -0.125, -0.125);
    glVertex3f(0.25, -0.125, -0.125);
    glVertex3f(0.25, -0.125, 0.125);
    glVertex3f(-0.25, -0.125, 0.125);
    
    glEnd();
}



void moveBoat(boat_t* boat) {
    glPushMatrix();
    glTranslatef(boat->position.x, boat->position.y, boat->position.z);
    glRotatef(boat->rotation_z + 90, 0, 0, 1);
    glRotatef(boat->rotation_y, 1, 0, 0);
    glRotatef(boat->centre_rot, 0, 1, 0);
    drawBoat();
    glPopMatrix();
    
    int size = (int)boats_proj.size();
    for (int i = 0; i < size; i++) {
        drawProjectile(*boats_proj[i], true);
        drawTrajectoryNumerical(*boats_proj[i]);
        
    }
}

void drawTurret(){
    glPushMatrix();
    glTranslatef(0.0, 2.5, 0.0);
    glRotatef(global.turret_angle, 0, 1, 0);
    glPushMatrix();
    glRotatef(90, 1.0, 0.0, 0.0);
    int parts = 12;
    float r = 1, h = 1;
    gluCylinder(base, r, r, h, parts, 2);
    glPopMatrix();
    
    float angle = (2.0 * M_PI) / parts;
    
    for (int i = 0; i < parts; i++) {
        float x = r * cosf(angle * i);
        float z = r * sinf(angle * i);
        
        pos = { x, 0.0, z };
        base_top[i] = pos;
    }
    
    glBegin(GL_POLYGON);
    glNormal3f(0, 1.0, 0);
    for (int i = 0; i < parts; i++) {
        int index = parts - i - 1;
        glVertex3f(base_top[index].x, base_top[index].y, base_top[index].z);
    }
    glEnd();
    
    glPushMatrix();
    glTranslatef(0.0, 0.25, 0.0);
    glBegin(GL_QUADS);
    //    TODO draw resi of the canon
    glNormal3f(0, 0, 1);
    glVertex3f(-.5, -0.25, 0.25);
    glVertex3f(.5, -0.25, 0.25);
    glVertex3f(0.5, 0.25, 0.25);
    glVertex3f(-0.5, 0.25, 0.25);
    
    glNormal3f(1, 0, 0);
    glVertex3f(0.5, -0.25, 0.25);
    glVertex3f(0.5, -0.25, -0.25);
    glVertex3f(0.5, 0.25, -0.25);
    glVertex3f(0.5, 0.25, 0.25);
    
    glNormal3f(0, 0, -1);
    glVertex3f(0.5, -0.25, -0.25);
    glVertex3f(-0.5, -0.25, -0.25);
    glVertex3f(-0.5, 0.25, -0.25);
    glVertex3f(0.5, 0.25, -0.25);
    
    glNormal3f(-1, 0, 0);
    glVertex3f(-0.5, -0.25, -0.25);
    glVertex3f(-0.5, -0.25, 0.25);
    glVertex3f(-0.5, 0.25, 0.25);
    glVertex3f(-0.5, 0.25, -0.25);
    glEnd();
    
    glPushMatrix();
    glTranslatef(0, 0.25, 0);
    glPushMatrix();
    glRotatef(-global.turret_angle, 0, 1, 0);
    
    drawTrajectoryNumerical(*projectile);
    
    int size = (int)island_proj.size();
    for (int i = 0; i < size; i++) {
        drawTrajectoryNumerical(*island_proj[i]);
        drawProjectile(*island_proj[i], false);
    }
    
    glColor3f(1.0, 1.0, 1.0);
    glPopMatrix();
    glRotatef(global.cannon_angle, 1, 0, 0);
    glTranslatef(0, 0, -0.875);
    gluCylinder(cannon, 0.125, 0.125, 0.75, 6, 1);
    
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.5, 0.25, 0);
    glRotatef(90, 0, 1, 0);
    gluCylinder(turret, 0.25, 0.25, 1, 12, 4);
    glPopMatrix();
    
    float short_l = 0.25 * sinf(M_PI / 6.0);
    float long_l = 0.25 * cosf(M_PI / 6.0);
    float d = 0.25;
    
    glPushMatrix();
    glTranslatef(-0.5, d, 0);
    glRotatef(90, 0, 1, 0);
    glRotatef(90, 1, 0, 0);
    glBegin(GL_POLYGON);
    glNormal3f(0, 1, 0);
    glVertex3f(-d, 0, 0);
    glVertex3f(-long_l, 0, -short_l);
    glVertex3f(-short_l, 0, -long_l);
    glVertex3f(0.0, 0, -d);
    glVertex3f(short_l, 0, -long_l);
    glVertex3f(long_l, 0, -short_l);
    glVertex3f(d, 0, 0);
    glEnd();
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.5, d, 0);
    glRotatef(90, 0, 1, 0);
    glRotatef(90, 1, 0, 0);
    glBegin(GL_POLYGON);
    glNormal3f(0, 1, 0);
    glVertex3f(d, 0, 0);
    glVertex3f(long_l, 0, -short_l);
    glVertex3f(short_l, 0, -long_l);
    glVertex3f(0.0, 0, -d);
    glVertex3f(-short_l, 0, -long_l);
    glVertex3f(-long_l, 0, -short_l);
    glVertex3f(-d, 0, 0);
    glEnd();
    glPopMatrix();
    
    glPopMatrix();
    glPopMatrix();
}

void drawIsland(){
    global.textures ? glBindTexture(GL_TEXTURE_2D, ground) : glBindTexture(GL_TEXTURE_2D, 0);
    glBegin(GL_QUAD_STRIP);
    
    float v = 0.0, parts = 12.0, curr_part = 0.0, dv = 0.0;
    int du = 1;
    
    
    float tex_centre = 0.5;
    float island_tex_ratio = 0.5 / 3;
    int max = island_base.size();
    for (int i = 0; i < max; i++) {
        int index = i;
        glNormal3f(island_normals[index].x, island_normals[index].y, island_normals[index].z);
        glTexCoord2f( 0.0 + 1.0 / parts * curr_part, v);
        glVertex3f(island_base[index].x, island_base[index].y, island_base[index].z);
        
        
        if (du % 2 == 0.0)
            curr_part++;
        
        dv == 0.25 ? dv = -0.25 : dv = 0.25;
        du++;
        v += dv;
    }
    
    glNormal3f(island_normals[0].x, island_normals[0].y, island_normals[0].z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(island_base[0].x, island_base[0].y, island_base[0].z);
    
    glNormal3f(island_normals[1].x, island_normals[1].y, island_normals[1].z);
    glTexCoord2f(1.0, 0.25);
    glVertex3f(island_base[1].x, island_base[1].y, island_base[1].z);
    
    glEnd();
    
    glBegin(GL_POLYGON);
    for (int i = 0; i < max / 2; i++) {
        int index = max - 1 - i * 2;
        
        glNormal3f(island_normals[index].x, island_normals[index].y, island_normals[index].z);
        glTexCoord2f(tex_centre + island_base[i * 2].x * island_tex_ratio, tex_centre + island_base[i * 2].z * island_tex_ratio);
        glVertex3f(island_base[index].x, island_base[index].y, island_base[index].z);
    }
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void drawNormals() {
    for (int j = 0; j <= space.n; j++) {
        for (int i = 0; i <= space.n; i++) {
            glDisable(GL_LIGHTING);
            glBegin(GL_LINES);
            glColor3f(0, 1, 0);
            
            glVertex3f(grid[j][i].x, grid[j][i].y, grid[j][i].z);
            glVertex3f(normals[j][i].x + grid[j][i].x, normals[j][i].y + grid[j][i].y, normals[j][i].z + grid[j][i].z);
            
            glColor3f(1, 1, 0);
            glVertex3f(grid[j][i].x, grid[j][i].y, grid[j][i].z);
            glVertex3f(tangents[j][i].x + grid[j][i].x, tangents[j][i].y + grid[j][i].y, tangents[j][i].z + grid[j][i].z);
            glEnd();
            
            if (global.lighting)
                glEnable(GL_LIGHTING);
            glColor3f(1, 1, 1);
        }
    }
    
    for (int i = 0; i < island_base.size(); i++) {
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        glColor3f(0, 1, 0);
        
        glVertex3f(island_base[i].x, island_base[i].y, island_base[i].z);
        glVertex3f(island_normals[i].x, island_normals[i].y, island_normals[i].z);
        
        glEnd();
        if (global.lighting)
            glEnable(GL_LIGHTING);
        glColor3f(1, 1, 1);
    }
}

void calculateSinWave(bool dvs) {
    float xStep = (space.x_start * 2.0f) / (float)space.n;
    float zStep = (space.z_start * 2.0f) / (float)space.n;
    float z, x;
    float dx, dz;
    
    //    First combination of x and z wave
    vec3f temp1[MAX_N][MAX_N];
    vec3f norm1[MAX_N][MAX_N];
    vec3f tan1[MAX_N][MAX_N];
    
    vec3f temp2[MAX_N][MAX_N];
    vec3f norm2[MAX_N][MAX_N];
    vec3f tan2[MAX_N][MAX_N];
    
    for (int j = 0; j <= space.n; j++) {
        z = space.z_start - (j * zStep);
        for (int i = 0; i <= space.n; i++) {
            x = -space.x_start + i * xStep;
            float y = calculateY(sw1, sw3, x, z);
            pos = { x, y, z};
            temp1[j][i] = pos;
            
            if (dvs){
                dx = calculate_dx(sw1, sw3, x, z);
                dz = calculate_dz(sw1, sw3, x, z);
                norm1[j][i] = { dx, 1.0, dz };
                
                tan1[j][i] = { 1.0, dx, 0.0 };
            }
        }
    }
    
    //    Second combination of x wave and z wave
    for (int j = 0; j <= space.n; j++) {
        z = space.z_start - (j * zStep);
        for (int i = 0; i <= space.n; i++) {
            x = -space.x_start + i * xStep;
            float y = calculateY(sw2, sw4, x, z);
            pos = { x, y, z};
            temp2[j][i] = pos;
            
            if (dvs){
                dx = calculate_dx(sw2, sw4, x, z);
                dz = calculate_dz(sw2, sw4, x, z);
                norm2[j][i] = { dx, 1.0, dz };
                
                tan2[j][i] = { 1.0, dx, 0.0 };
            }
        }
    }
    
    //    Adding values previously calculated and putting them into the grid array
    for (int j = 0; j <= space.n; j++) {
        for (int i = 0; i <= space.n; i++) {
            
            grid[j][i] = { temp2[j][i].x, temp1[j][i].y + temp2[j][i].y, temp2[j][i].z };
            
            normals[j][i] = { -(norm1[j][i].x + norm2[j][i].x), norm1[j][i].y, -(norm1[j][i].z + norm2[j][i].z) };
            
            tangents[j][i] = { tan1[j][i].x, tan1[j][i].y + tan2[j][i].y, tan1[j][i].z + tan2[j][i].z };
        }
    }
}


void drawGrid(){
    for (int i = 0; i < space.n; i++) {
        bool next_col = true;
        int p2 = 0;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= space.n; j++) {
            glNormal3f(normals[i + 1][p2].x, normals[i + 1][p2].y,  normals[i + 1][p2].z);
            glVertex3f(grid[i + 1][p2].x, grid[i + 1][p2].y, grid[i + 1][p2].z);
            
            glNormal3f(normals[i][p2].x, normals[i][p2].y,  normals[i][p2].z);
            glVertex3f(grid[i][p2].x, grid[i][p2].y, grid[i][p2].z);
            p2 += 1;
            next_col = !next_col;
        }
        glEnd();
    }
}

void drawHealthBar(int max, int curr, float x, float y) {
    float lengthOfPoint = 200 / float(max);
    glBegin(GL_QUADS);
    glVertex3f(x, y - 20, 0.0);
    glVertex3f(x + float(curr)*lengthOfPoint, y - 20, 0.0);
    glVertex3f(x + float(curr)*lengthOfPoint, y , 0.0);
    glVertex3f(x, y, 0.0);
    glEnd();
}

void displayOSD()
{
    char buffer[30];
    char *bufp;
    int w, h;
    
    glDisable(GL_LIGHTING);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    
    /* Set up orthographic coordinate system to match the
     window, i.e. (0,0)-(w,h) */
    w = glutGet(GLUT_WINDOW_WIDTH);
    h = glutGet(GLUT_WINDOW_HEIGHT);
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor3f(1, 1, 0);
    drawHealthBar(100, global.island_health, 35, h - 20);
    
    glColor3f(1, 1, 1);
    
    glColor3f(1.0, 0.0, 0.0);
    glRasterPos2i(35, h - 70);
    snprintf(buffer, sizeof buffer, "Score: %d", global.points);
    for (bufp = buffer; *bufp; bufp++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *bufp);
    glFlush();
    glColor3f(1, 1, 1);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    
    /* Pop projection */
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void display(void)
{
    GLenum err;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_TEXTURE_2D);
    if (global.lighting)
        glEnable(GL_LIGHTING);
    
    glEnable(GL_NORMALIZE);
    
    GLfloat light[] = { 0.0, 1.0, 0.0, 0.0};
    GLfloat cyan[] = { 0.0, 1, 1.0, 1.0};
    GLfloat water_blue[] = { 0.0, 0.5, 1.0, .75};
    GLfloat white[] = { 1.0, 1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, light);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_AMBIENT, white);
    setCamera();
    glEnable(GL_LIGHT0);
    
    glEnable(GL_DEPTH_TEST);
    
    if (global.wiretframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT, GL_FILL);
    
    drawAxes(2.0);
    
    if (global.normals)
        drawNormals();
    drawIsland();
    
    int size = (int)boats.size();
    for (int i = 0; i < size; i++) {
        moveBoat(boats[i]);
    }
    
    glDisable(GL_TEXTURE_2D);
    
    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_DIFFUSE, cyan);
    drawTurret();
    glPopMatrix();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, water_blue);
    drawGrid();
    glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
    
    displayOSD();
    glutSwapBuffers();
    
    // Check for errors
    while ((err = glGetError()) != GL_NO_ERROR)
        printf("%s\n",gluErrorString(err));
}

void stopRotation() {
    data.xRot = 0.0f;
    data.yRot = 0.0f;
    global.rotateCamera = false;
}

void stopZoom() {
    data.lastPosition = -1.0f;
    global.zoom = false;
}

void mouseEvent(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        data.lastX = -1.0f;
        data.lastY = -1.0f;
        global.rotateCamera = true;
        stopZoom();
    }
    
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP){
        stopRotation();
    }
    
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
        data.lastPosition = -1.0f;
        global.zoom = true;
        stopRotation();
    }
    
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP){
        stopZoom();
    }
    
    glutPostRedisplay();
}

void mouseMotion(int x, int y) {
    if (global.rotateCamera) {
        if (data.lastY == -1.0f)
            data.lastY = x;
        data.yRot = x - data.lastY;
        data.lastY = x;
        
        
        if (data.lastX == -1.0f)
            data.lastX = y;
        data.xRot = y - data.lastX;
        data.lastX = y;
        
    }
    
    if (global.zoom){
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
                global.textures = !global.textures;
                break;
            case 'l':
                if (!global.finished)
                    global.lighting = !global.lighting;
                break;
            case 'h':
                if (!global.finished)
                    global.animation = !global.animation;
                break;
            case 'a':
                if (global.cannon_angle < 90 && !global.finished)
                    global.cannon_angle++;
                break;
            case 'd':
    
                if (global.cannon_angle > 0 && !global.finished)
                    global.cannon_angle--;
                break;
            case ' ':
                if (global.allowed_to_fire && !global.finished) {
                    global.allowed_to_fire = false;
                    //                TODO fire multiple projectiles
                    global.last_fired = glutGet(GLUT_ELAPSED_TIME);
                    //                TODO Change this to the cannon projectile
                    projectile->v.x = projectile->v0.x;
                    projectile->v.y = projectile->v0.y;
                    projectile->v.z = projectile->v0.z;
                    
                    projectile->r.x = projectile->r0.x;
                    projectile->r.y = projectile->r0.y;
                    projectile->r.z = projectile->r0.z;
                    
                    state* new_proj = new state;
                    initProjectile(*new_proj, *projectile);
                    island_proj.push_back(new_proj);
                }
                break;
            case 'w':
                if (global.speed < 14 && !global.finished)
                    global.speed += 0.5;
                break;
            case 's':
                if (global.speed > 5 && !global.finished)
                    global.speed -= 0.5;
                break;
            case '+':
                if (space.n * 2 < MAX_N - 1)
                    space.n *= 2;
                break;
            case '-':
                if (space.n > 20)
                    space.n /= 2;
                break;
            default:
                break;
        
    }
    glutPostRedisplay();
}

void specialInput(int key, int x, int y)
{
    if(!global.finished) {
        switch (key) {
            case GLUT_KEY_RIGHT:
                global.turret_angle -= 5;
                camera.yRot += 5;
                break;
            case GLUT_KEY_LEFT:
                global.turret_angle += 5;
                camera.yRot -= 5;
                
                break;
            default:
                break;
        }
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

// Idle callback for animation
void update(void)
{
    if (global.island_health <= 0){
        global.finished = true;
        global.animation = false;
    }
    
    float t, dt;
    t = glutGet(GLUT_ELAPSED_TIME);
    t /= 5000.0;
    
    if (global.rotateCamera){
        camera.xRot = camera.lastX + data.xRot;
        camera.lastX = camera.xRot;
        
        camera.yRot = camera.lastY + data.yRot;
        camera.lastY = camera.yRot;
    }
    
    if (global.zoom) {
        camera.position = camera.lastPosition + data.position;
        camera.lastPosition = camera.position;
    }
    
    //    Chnage in time
    dt = t - t_last;
    
    if ((glutGet(GLUT_ELAPSED_TIME) - global.last_spawn_boat_t > 10000 || global.last_spawn_boat_t == 0.0) && global.animation) {
        global.last_spawn_boat_t = glutGet(GLUT_ELAPSED_TIME);
        boat_t* boat = new boat_t;
        initBoat(boat);
        boats.push_back(boat);
    }
    
    t_last = t;
    convertPolToCar(*projectile);
    
    calculateSinWave(true);
    
    if (global.animation) {
        global.animationX += dt;
        
        //        Fires a missile every 5 seconds
        int size = (int)boats.size();
        for (int i = 0; i < size; i++) {
            updateBoat(boats[i], dt);
            if (glutGet(GLUT_ELAPSED_TIME) - boats[i]->last_fired > 5000.0 ){
                boats[i]->last_fired = glutGet(GLUT_ELAPSED_TIME);
                state *projectile = new state;
                initProjectile(*projectile, boats[i]->projectile);
                boats_proj.push_back(projectile);
            }
        }
        
        //        Checks for collision of a missle and island
        size = (int)boats_proj.size();
        int toDelete = -1;
        for (int i = 0; i < size; i++) {
            updateProjectileStateNumerical(boats_proj[i], dt);
            if (checkCollision(*boats_proj[i])) {
                global.island_health--;
                toDelete = i;
            }
        }
        if (toDelete != -1) {
            state* proj = boats_proj[toDelete];
            boats_proj.erase(boats_proj.begin() + toDelete);
            delete proj;
        }
        
        //        Checks for island missiles' collision with a boat
        toDelete = -1;
        size = (int)island_proj.size();
        for (int i = 0; i < size; i++) {
            updateProjectileStateNumerical(island_proj[i], dt);
            
            if (checkIslandProjCollision(*island_proj[i])) {
                global.points++;
                toDelete = i;
            }
            if (island_proj[i]->r.y < -2.0 && toDelete == -1) {
                toDelete = i;
            }
        }
        if (toDelete != -1) {
            state* proj = island_proj[toDelete];
            island_proj.erase(island_proj.begin() + toDelete);
            delete proj;
        }
    }
    
    if (glutGet(GLUT_ELAPSED_TIME) - global.last_fired > 3000 && !global.allowed_to_fire && global.animation)
        global.allowed_to_fire = true;
    
    glutPostRedisplay();
}

void init(){
    srand(time(nullptr));
    initBoat(boat);
    initIsland();
}

uint32_t loadTexture(const char* filename, int i) {
    int width, height, components;
    unsigned char* data = stbi_load(filename, &width, &height, &components, STBI_rgb);
    
    glPushAttrib(GL_TEXTURE_BIT);
    
    unsigned int id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    //      glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    
    glPopAttrib();
    
    return id;
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
    glutInitWindowPosition(200, 200);
    glutCreateWindow("Island Defence");
    
    init();
    
    glutSpecialFunc(specialInput);
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
    const char* file2 = "zneg.png";
    const char* file3 = "xpos.png";
    const char* file4 = "xneg.png";
    const char* file5 = "ypos.png";
    const char* file6 = "yneg.png";
    
    //    zBack = loadTexture(file1);
    ground = loadTexture(argv[1], 1);
    zPos = loadTexture(file1, 2);
    zNeg = loadTexture(file2, 3);
    xPos = loadTexture(file3, 4);
    xNeg = loadTexture(file4, 5);
    yPos = loadTexture(file5, 6);
    yNeg = loadTexture(file6, 7);
    
    if (!ground || !zPos || !xPos || !zNeg || !xNeg|| !yNeg || !yPos)  {
        printf("No texture created; exiting.\n");
        return EXIT_FAILURE;
    }
    
    glutMainLoop();
}
