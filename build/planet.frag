#version 330 core
#define NUM_LIGHTS 2

struct lightSource {    
    vec3 position;
    vec3 colour;
};

out vec4 FragColour;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
// lighting
uniform vec3 viewPos;

uniform lightSource lights[NUM_LIGHTS];
// texture    
uniform sampler2D ourTexture;

void main()
{   
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    // final calc
    vec3 result = vec3(0.0f);
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        result += calcPhong(lights[0], norm, FragPos, viewDir); 
    }
    vec3 out = result * texture(ourTexture, TexCoord).rgb;
    FragColour = vec4(out, 1.0);
}

vec3 calcPhong(lightSource light, vec3 norm, vec3 fragPos, vec3 viewDir)
{
    // ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light.colour;

    // diffusion lighting
    float diffusionStrength = 0.1;
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), diffusionStrength);
    vec3 diffuse = diff * light.colour;

    // specular lighting
    float specularStrength = 0.65;
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * light.colour;

    return (ambient + diffuse + specular);
}