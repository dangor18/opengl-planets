#version 330 core

in vec3 position;

uniform mat4 transform;
uniform mat4 MVP;

void main()
{
    gl_Position = MVP * transform * vec4(position,1.0f);
}