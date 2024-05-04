#include <iostream>
#include <string>
#include <stdio.h>

#include "SDL.h"
#include <GL/glew.h>

#include "glwindow.h"
#include "geometry.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// window
SDL_Window* sdlWin;

// buffers
GLuint vao;
GLuint vertexBuffer;
GLuint normBuffer;
GLuint textureBuffer;

// shaders
GLuint planet_shader;
GLuint sun_shader;

// textures
GLuint sun_texture;
GLuint earth_texture;
GLuint moon_texture;

// object vertex count
int vertexCount;

// speed variables
float earth_speed = 1.0f;
float moon_speed = 1.0f;
// angles
float theta = 0;
float beta = 0;

// when press p set to 0
int p = 1;

// camera variables
float pitch, yaw, roll = 0.0f;

// view matrix
glm::mat4 view;

const char* glGetErrorString(GLenum error)
{
    switch(error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char* label="Unlabelled Error Checkpoint", bool alwaysPrint=false)
{
    GLenum error = glGetError();
    if(alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
    FILE* shaderFile = fopen(shaderFilename, "r");
    if(!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char* shaderText = new char[shaderSize+1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}

GLuint loadShaderProgram(const char* vertShaderFilename,
                       const char* fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

OpenGLWindow::OpenGLWindow()
{
}


void OpenGLWindow::initGL()
{
    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow("OpenGL Prac 1",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480, SDL_WINDOW_OPENGL);
    if(!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if(glewInitResult != GLEW_OK)
    {
        const GLubyte* errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    sun_shader = loadShaderProgram("sun.vert", "sun.frag");
    planet_shader = loadShaderProgram("planet.vert", "planet.frag");
    glUseProgram(planet_shader);
    
    // Load the model that we want to use and buffer the vertex attributes
    GeometryData geom;
    geom.loadFromOBJFile("sphere_correct.obj");
    void* vertices = geom.vertexData();
    void* normals = geom.normalData();
    void* texCoords = geom.textureCoordData();
    vertexCount = geom.vertexCount();
    
    // locations
    int vertexLoc = 0;
    int normLoc = 1;
    int texLoc = 2;

    // VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // VBO
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 3*vertexCount*sizeof(float), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(vertexLoc);
    // Normals
    glGenBuffers(1, &normBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
    glBufferData(GL_ARRAY_BUFFER, 3*vertexCount*sizeof(float), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);
    glEnableVertexAttribArray(normLoc);
    // TBO
    glGenBuffers(1, &textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glBufferData(GL_ARRAY_BUFFER, 2*vertexCount*sizeof(float), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(texLoc);
    
    // create texture objs
    glGenTextures(1, &sun_texture); 
    glGenTextures(1, &earth_texture);
    glGenTextures(1, &moon_texture);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load and generate the texture from each file - taken from learnopengl.com/Getting-started/Textures
    glBindTexture(GL_TEXTURE_2D, sun_texture);
    load_image("Suns/diffuse0.jpg");

    glBindTexture(GL_TEXTURE_2D, earth_texture);
    load_image("Earth/diffuse.png");

    glBindTexture(GL_TEXTURE_2D, moon_texture);
    load_image("Moon/diffuse.png");

    //PROJ
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float) 640/480, 0.1f, 100.0f);
    
    glUseProgram(planet_shader);
    unsigned int projLoc = glGetUniformLocation(planet_shader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    // light uniforms
    unsigned int sunLightPosLoc = glGetUniformLocation(planet_shader, "lightPos");
    glUniform3f(sunLightPosLoc, 0.0f, 0.0f, 0.0f);
    unsigned int sunLightColLoc = glGetUniformLocation(planet_shader, "lightColor");
    glUniform3f(sunLightColLoc, 1.0f, 1.0f, 1.0f);

    glUseProgram(sun_shader);
    projLoc = glGetUniformLocation(sun_shader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    glPrintError("Setup complete", true);
}

void OpenGLWindow::render()
{
    // VIEW
    //view = glm::mat4(1.0f);
    glm::vec3 lookat = glm::vec3(0.0f);
    glm::vec3 camera = glm::vec3(0.0f, 0.0f, 8.0f);
    glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f);
    /*
    glm::mat4 camera_rotx = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));   // rotate about the X-axis
    glm::mat4 camera_roty = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));     // rotate about the Y-axis
    glm::mat4 camera_rotz = glm::rotate(glm::mat4(1.0f), glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));    // rotate about the Z-axis
    glm::mat4 camera_rotation = camera_rotz * camera_roty * camera_rotx;

    camera = glm::vec3(camera_rotation * glm::vec4(camera, 1.0f));  // calculate new camera position
    */
    view = glm::lookAt(camera, lookat, up);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use sun shader
    glUseProgram(sun_shader);
    unsigned int viewLoc = glGetUniformLocation(sun_shader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    unsigned int transformLoc = glGetUniformLocation(sun_shader, "model");

    // Draw Sun at (0, 0, 0)
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    glBindTexture(GL_TEXTURE_2D, sun_texture);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // calc rotation angles
    theta += p*earth_speed;
    beta += p*moon_speed;

    // use planet shader
    glUseProgram(planet_shader);
    viewLoc = glGetUniformLocation(planet_shader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    transformLoc = glGetUniformLocation(planet_shader, "model");

    // Draw Earth
    glm::mat4 earth_r = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 earth_t = glm::translate(glm::mat4(1.0f), glm::vec3(2.2f, 0.0f, 0.0f));
    glm::mat4 earth_s = glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f));
    glm::mat4 earth_trans = earth_r * earth_t * earth_s;
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(earth_trans));
    // lighting
    unsigned int MVNloc = glGetUniformLocation(planet_shader, "MVN");
    glUniformMatrix3fv(MVNloc, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(glm::mat3(view * earth_trans)))));
    unsigned int viewPosLoc = glGetUniformLocation(planet_shader, "viewPos");
    glUniformMatrix3fv(viewPosLoc, 1, GL_FALSE, glm::value_ptr(camera));
    // bind to correct texture
    glBindTexture(GL_TEXTURE_2D, earth_texture);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // Draw Moon
    glm::mat4 moon_r = glm::rotate(glm::mat4(1.0f), glm::radians(beta), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 moon_t = glm::translate(glm::mat4(1.0f), glm::vec3(1.8f, 0.0f, 0.0f));
    glm::mat4 moon_s = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
    glm::mat4 moon_trans = earth_trans * moon_r * moon_t * moon_s;
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(moon_trans));
    // bind to correct texture
    glBindTexture(GL_TEXTURE_2D, moon_texture);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // Swap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    SDL_GL_SwapWindow(sdlWin);
}

void OpenGLWindow::load_image(const char* file_name)
{
    // get extension (png or jpg)
    std::string fileName(file_name);
    std::string extension = fileName.substr(fileName.find('.') + 1);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(file_name, &width, &height, &nrChannels, 0);

    // first check data was loaded correctly
    if (data)
    {
        if (extension == "jpg")
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        if (extension == "png")
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load image: " << file_name << std::endl;
    }
    stbi_image_free(data);
}

// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    // A list of keycode constants is available here: https://wiki.libsdl.org/SDL_Keycode
    // Note that SDL provides both Scancodes (which correspond to physical positions on the keyboard)
    // and Keycodes (which correspond to symbols on the keyboard, and might differ across layouts)
    if(e.type == SDL_KEYDOWN)
    {
        if(e.key.keysym.sym == SDLK_ESCAPE)
        {
            return false;
        }
        /*
        if(e.key.keysym.sym == SDLK_UP)
        {
            earth_speed+=0.1;
        }

        if(e.key.keysym.sym == SDLK_DOWN)
        {
            earth_speed <= 1 ? earth_speed = 1 : earth_speed-=0.1;
        }

        if(e.key.keysym.sym == SDLK_LEFT)
        {
            moon_speed <= 1 ? moon_speed = 1 : moon_speed-=0.1;
        }

        if(e.key.keysym.sym == SDLK_RIGHT)
        {
            moon_speed+=0.1;
        }
        */

        if(e.key.keysym.sym == SDLK_UP)
        {
            pitch-=1.0f;
            // added to prevent look at flipping
            if(pitch < -89.0f)
                pitch = -89.0f;
        }

        if(e.key.keysym.sym == SDLK_DOWN)
        {
            pitch+=1.0f;
            // added to prevent look at flipping
            if(pitch > 89.0f)
                pitch = 89.0f;
        }

        if(e.key.keysym.sym == SDLK_LEFT)
        {
            yaw-=1.0f;
        }

        if(e.key.keysym.sym == SDLK_RIGHT)
        {
            yaw+=1.0f;
        }

        if(e.key.keysym.sym == SDLK_d)
        {
            roll+=1.0f;
        }

        if(e.key.keysym.sym == SDLK_f)
        {
            roll-=1.0f;
        }

        // reset key
        if(e.key.keysym.sym == SDLK_r)
        {
            pitch = roll = yaw = 0.0f;
        }

        if(e.key.keysym.sym == SDLK_p)
        {
            if (p) { p = 0; }
            else { p = 1; }
        }
    }
    return true;
}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &textureBuffer);
    glDeleteBuffers(1, &sun_texture);
    glDeleteBuffers(1, &earth_texture);
    glDeleteBuffers(1, &moon_texture);

    glDeleteVertexArrays(1, &vao);
    SDL_DestroyWindow(sdlWin);
}
