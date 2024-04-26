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
    GLuint planet_shader;
    GLuint sun_shader;
    GLuint vertexBuffer;
};

#endif
