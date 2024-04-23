#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>

#include "geometry.h"

class OpenGLWindow
{
public:
    OpenGLWindow();

    void initGL();
    void render();
    bool handleEvent(SDL_Event e);
    void cleanup();

private:
    SDL_Window* sdlWin;
    int vertexCount;
    float time = 0.0f;
    float c1 = 1.0f;
    float c2 = 1.0f;
    GLuint vao;
    GLuint shader;
    GLuint vertexBuffer;
};

#endif
