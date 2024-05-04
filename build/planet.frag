#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
// lighting
uniform vec3 lightPos;  
uniform vec3 lightColor;  
uniform vec3 viewPos;
// texture    
uniform sampler2D ourTexture;

void main()
{  	
    // ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffusion lighting
    float diffusionStrength = 0.1;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), diffusionStrength);
    vec3 diffuse = diff * lightColor;

    // specular lighting
    float specularStrength = 0.8;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;  
    
    // final calc
    vec3 result = (diffuse) * texture(ourTexture, TexCoord).rgb;
    FragColor = vec4(result, 1.0);
}