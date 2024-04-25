#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>
#include <string>

#include "geometry.h"

class OpenGLWindow
{
public:
    OpenGLWindow();

    void initGL();
    void render();
    unsigned char* loadImage(const char* file_name);
    bool handleEvent(SDL_Event e);
    void cleanup();

private:
    SDL_Window* sdlWin;
    GLuint vao;
    GLuint shader;
    GLuint vertexBuffer;
    GLuint textureBuffer;
    GLuint sun_texture;
    GLuint earth_texture;
    GLuint moon_texture;
};

#endif
