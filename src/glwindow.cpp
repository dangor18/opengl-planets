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
float win_width = 1280.0f;
float win_height = 720.0f;

// buffers
GLuint sphereVAO;
GLuint skyboxVAO;
GLuint sphereVBO;
GLuint skyboxVBO;
GLuint normBuffer;
GLuint textureBuffer;

// shaders
GLuint planet_shader;
GLuint light_shader;
GLuint skybox_shader;

// textures
GLuint sun_texture;
GLuint earth_texture;
GLuint moon_texture;
GLuint mars_texture;
GLuint venus_texture;
GLuint skybox_texture;

// object vertex count
int vertexCount;

// speed variables
float earth_speed = 0.005f;
float moon_speed = 0.01f;
// angles
float theta = 0;
float beta = 0;

// when press p set to 0
int p = 1;

// camera variables
float pitch, yaw, roll = 0.0f;

// view matrix
glm::mat4 view;

// movable light defaults
float light1_rad = 0.75f;
glm::vec3 light1Colour = glm::vec3(1.0f, 0.0f, 0.0f);
float alpha = 0;
float light_speed = 0.005f;
bool light1_active = false;

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
                              win_width, win_height, SDL_WINDOW_OPENGL);
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
    light_shader = loadShaderProgram("light.vert", "light.frag");
    planet_shader = loadShaderProgram("planet.vert", "planet.frag");
    skybox_shader = loadShaderProgram("skybox.vert", "skybox.frag");
    glUseProgram(planet_shader);
    
    // Load the model that we want to use and buffer the vertex attributes
    GeometryData geom;
    geom.loadFromOBJFile("sphere_correct.obj");
    void* sphere_vertices = geom.vertexData();
    void* sphere_normals = geom.normalData();
    void* sphere_texCoords = geom.textureCoordData();
    vertexCount = geom.vertexCount();
    
    // skybox vertices
    float skybox_vertices[] = 
    {        
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };
    
    // locations
    int vertexLoc = 0;
    int normLoc = 1;
    int texLoc = 2;

    // sphere VAO
    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);
    // sphere VBO
    glGenBuffers(1, &sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, 3*vertexCount*sizeof(float), sphere_vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(vertexLoc);
    // Normals
    glGenBuffers(1, &normBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normBuffer);
    glBufferData(GL_ARRAY_BUFFER, 3*vertexCount*sizeof(float), sphere_normals, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);
    glEnableVertexAttribArray(normLoc);
    // TBO
    glGenBuffers(1, &textureBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
    glBufferData(GL_ARRAY_BUFFER, 2*vertexCount*sizeof(float), sphere_texCoords, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(texLoc);
    // Tangents

    // Bitangents

    // skybox VAO
    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);
    // skybox VBO
    glGenBuffers(1, &skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, 3*36*sizeof(float), skybox_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(vertexLoc);

    // create texture objs
    glGenTextures(1, &sun_texture); 
    glGenTextures(1, &earth_texture);
    glGenTextures(1, &moon_texture);
    glGenTextures(1, &mars_texture);
    glGenTextures(1, &venus_texture);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load and generate the texture from each file - taken from learnopengl.com/Getting-started/Textures
    std::cout << "Loading textures..." << std::endl;
    stbi_set_flip_vertically_on_load(true);
    glBindTexture(GL_TEXTURE_2D, sun_texture);
    load_image("Suns/diffuse0.jpg");

    glBindTexture(GL_TEXTURE_2D, earth_texture);
    load_image("Earth/diffuse.png");

    glBindTexture(GL_TEXTURE_2D, moon_texture);
    load_image("Moon/diffuse.png");

    glBindTexture(GL_TEXTURE_2D, mars_texture);
    load_image("Mars/diffuse.jpg");

    glBindTexture(GL_TEXTURE_2D, venus_texture);
    load_image("Venus/diffuse.jpg");

    // load skybox
    vector<std::string> faces
    {
        "bkg1_right.jpg",
        "bkg1_left.jpg",
        "bkg1_top.jpg",
        "bkg1_bot.jpg",
        "bkg1_front.jpg",
        "bkg1_back.jpg"
    };
    load_cubemap(faces);

    std::cout << "Texture loaded." << std::endl;

    //PROJ
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), win_width/win_height, 0.1f, 100.0f);
    
    // skybox 
    glUseProgram(skybox_shader);
    unsigned int projLoc = glGetUniformLocation(skybox_shader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    // planets
    glUseProgram(planet_shader);
    projLoc = glGetUniformLocation(planet_shader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    // light uniforms
    // SUN
    unsigned int sunLightPosLoc = glGetUniformLocation(planet_shader, "sunPos");
    glUniform3f(sunLightPosLoc, 0.0f, 0.0f, 0.0f);
    unsigned int sunLightColLoc = glGetUniformLocation(planet_shader, "sunColour");
    glUniform3f(sunLightColLoc, 1.0f, 1.0f, 0.8f);

    glUseProgram(light_shader);
    projLoc = glGetUniformLocation(light_shader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    glPrintError("Setup complete", true);
}

void OpenGLWindow::render()
{
    // VIEW
    glm::vec3 lookat = glm::vec3(0.0f);
    glm::vec3 camera = glm::vec3(0.0f, 0.0f, 8.0f);
    glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f);
    glm::mat4 camera_rotx = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));   // rotate about the X-axis
    glm::mat4 camera_roty = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));     // rotate about the Y-axis
    glm::mat4 camera_rotz = glm::rotate(glm::mat4(1.0f), glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));    // rotate about the Z-axis
    glm::mat4 camera_rotation = camera_rotz * camera_roty * camera_rotx;

    camera = glm::vec3(camera_rotation * glm::vec4(camera, 1.0f));  // calculate new camera position
    view = glm::lookAt(camera, lookat, up);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use skybox shader
    glUseProgram(skybox_shader);
    glBindVertexArray(skyboxVAO);
    unsigned int viewLoc = glGetUniformLocation(skybox_shader, "view");
    glm::mat4 skybox_view = glm::mat4(glm::mat3(view)); // remove translation from the view matrix
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(skybox_view));

    // DRAW SKYBOX
    glDepthMask(GL_FALSE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    
    // use light shader
    glUseProgram(light_shader);
    glBindVertexArray(sphereVAO);
    viewLoc = glGetUniformLocation(light_shader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    unsigned int transformLoc = glGetUniformLocation(light_shader, "model");

    // DRAW SUN
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1.0f), glm::vec3(0.7f, 0.7f, 0.7f))));
    glUniform1i(glGetUniformLocation(light_shader, "textureSwitch"), 1);
    glBindTexture(GL_TEXTURE_2D, sun_texture);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    float light1_x; float light1_z;
    // Draw light
    if (light1_active)
    {
        alpha += p*light_speed;
        light1_x = light1_rad * sin(alpha); light1_z = light1_rad * cos(alpha);
        glm::mat4 light1_t = glm::translate(glm::mat4(1.0f), glm::vec3(light1_x, 0.0f, light1_z));
        glm::mat4 light1_s = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f));
        glm::mat4 light1_trans = light1_t * light1_s;
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(light1_trans));
        glUniform1i(glGetUniformLocation(light_shader, "textureSwitch"), 0);
        glUniform3f(glGetUniformLocation(light_shader, "lightColour"), light1Colour.r, light1Colour.g, light1Colour.b);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
    // calc rotation angles
    theta += p*earth_speed;
    beta += p*moon_speed;

    // use planet shader
    glUseProgram(planet_shader);
    viewLoc = glGetUniformLocation(planet_shader, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    transformLoc = glGetUniformLocation(planet_shader, "model");

    // light 1
    if (light1_active)
    {
        unsigned int light1PosLoc = glGetUniformLocation(planet_shader, "light1Pos");
        glUniform3f(light1PosLoc, light1_x, 0.0f, light1_z);
        unsigned int light1ColLoc = glGetUniformLocation(planet_shader, "light1Colour");
        glUniform3f(light1ColLoc, light1Colour.r, light1Colour.g, light1Colour.b);
    } else
    {
        unsigned int light1ColLoc = glGetUniformLocation(planet_shader, "light1Colour");
        glUniform3f(light1ColLoc, 0.0f, 0.0f, 0.0f);
    }

    // DRAW EARTH
    glm::mat4 earth_t = glm::translate(glm::mat4(1.0f), glm::vec3(sin(theta) * 2.2f, cos(theta) * 2.2f, 0.0f));
    glm::mat4 earth_r = glm::rotate(glm::mat4(1.0f), glm::radians(theta*100), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 earth_s = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
    glm::mat4 earth_trans = earth_t * earth_r * earth_s;
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(earth_trans));
    // lighting
    unsigned int MVNloc = glGetUniformLocation(planet_shader, "MVN");
    glUniformMatrix3fv(MVNloc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(earth_trans)))));
    unsigned int viewPosLoc = glGetUniformLocation(planet_shader, "viewPos");
    glUniform3f(viewPosLoc, camera.x, camera.y, camera.z);
    // bind to correct texture
    glBindTexture(GL_TEXTURE_2D, earth_texture);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // DRAW MOON
    glm::mat4 moon_t = glm::translate(glm::mat4(1.0f), glm::vec3(sin(beta) * 1.75f, cos(beta) * 1.75f, 0.0f));
    glm::mat4 moon_r = glm::rotate(glm::mat4(1.0f), glm::radians(beta*100), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 moon_s = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
    glm::mat4 moon_trans = earth_t * earth_s * moon_t * moon_r * moon_s;
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(moon_trans));
    // lighting
    MVNloc = glGetUniformLocation(planet_shader, "MVN");
    glUniformMatrix3fv(MVNloc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(moon_trans)))));
    // bind to correct texture
    glBindTexture(GL_TEXTURE_2D, moon_texture);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // DRAW VENUS
    glm::mat4 venus_t = glm::translate(glm::mat4(1.0f), glm::vec3(sin(beta) * 1.2f, cos(beta) * 1.2f, 0.0f));
    glm::mat4 venus_r = glm::rotate(glm::mat4(1.0f), glm::radians(beta*100), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 venus_s = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
    glm::mat4 venus_trans = venus_t * venus_r * venus_s;
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(venus_trans));
    // lighting
    MVNloc = glGetUniformLocation(planet_shader, "MVN");
    glUniformMatrix3fv(MVNloc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(venus_trans)))));
    // bind to correct texture
    glBindTexture(GL_TEXTURE_2D, venus_texture);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    
    // DRAW MARS
    glm::mat4 mars_t = glm::translate(glm::mat4(1.0f), glm::vec3(sin(beta+90) * 3.0f, cos(beta+90) * 3.0f, 0.0f));
    glm::mat4 mars_r = glm::rotate(glm::mat4(1.0f), glm::radians(beta*100), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 mars_s = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
    glm::mat4 mars_trans = mars_t * mars_r * mars_s;
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(mars_trans));
    // lighting
    MVNloc = glGetUniformLocation(planet_shader, "MVN");
    glUniformMatrix3fv(MVNloc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(mars_trans)))));
    // bind to correct texture
    glBindTexture(GL_TEXTURE_2D, mars_texture);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    
    // Swap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    SDL_GL_SwapWindow(sdlWin);
}

void OpenGLWindow::load_image(std::string file_name)
{
    // get extension (png or jpg)
    std::string extension = file_name.substr(file_name.find('.') + 1);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);

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

void OpenGLWindow::load_cubemap(std::vector<std::string> faces)
{
    // generate and bind texture object
    glGenTextures(1, &skybox_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    int width, height, nrChannels;

    for (unsigned int i = 0; i < 6; i++)
    {
        std::string file_name = "Skybox/" + faces[i];
        unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
        // first check data was loaded correctly
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            std::cout << file_name << std::endl;
        }
        else
        {
            std::cout << "Failed to load cubemap image at path" << i << std::endl;
        }
        stbi_image_free(data);
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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
        
        // pause
        if(e.key.keysym.sym == SDLK_p)
        {
            if (p) { p = 0; }
            else { p = 1; }
        }

        // turn off light1
        if(e.key.keysym.sym == SDLK_o)
        {
            if (light1_active)
                light1_active = false;
            else
                light1_active = true;
        }
    }
    return true;
}

void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &textureBuffer);
    glDeleteBuffers(1, &sun_texture);
    glDeleteBuffers(1, &earth_texture);
    glDeleteBuffers(1, &moon_texture);

    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    SDL_DestroyWindow(sdlWin);
}
