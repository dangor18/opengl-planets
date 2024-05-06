#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform int textureSwitch; // 0 for no texture, 1 for texture
uniform vec3 lightColour;

void main()
{
    if (textureSwitch == 1) {
        FragColor = texture(ourTexture, TexCoord);
    } else {
        FragColor = vec4(lightColour, 1);
    }
}
