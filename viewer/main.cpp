#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#include "stb_image.h"


#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  if _WIN32
#    include <Windows.h>
#  endif
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glut.h>
#endif

#if 0
// Program uses the Simple OpenGL Image Library for loading textures: http://www.lonesock.net/soil.html
#include <SOIL.h>
#endif

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

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

  glutSwapBuffers();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("Texture viewer");
  glutDisplayFunc(display);

  if (argc < 2) {
    printf("Specify an image filename to display.\n");
    return EXIT_FAILURE;
  }

  texture = loadTexture(argv[1]);
  if (!texture)  {
    printf("No texture created; exiting.\n");
    return EXIT_FAILURE;
  }

  glutMainLoop();

  return EXIT_SUCCESS;
}

