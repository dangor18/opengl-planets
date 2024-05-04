#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>
#include <string>

#include "geometry.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class OpenGLWindow
{
public:
    OpenGLWindow();

    void initGL();
    void render();
    bool handleEvent(SDL_Event e);
    void cleanup();

    // loads image data
    void load_image(const char* file_name);
};

#endif
