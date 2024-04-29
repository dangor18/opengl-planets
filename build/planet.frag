#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
// lighting
uniform vec3 lightPos;  
uniform vec3 lightColor;  
// texture    
uniform sampler2D ourTexture;

void main()
{  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    vec3 result = diffuse * texture(ourTexture, TexCoord).xyz;
    FragColor = vec4(result, 1.0);
}