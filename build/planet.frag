#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
// lighting
uniform vec3 sunPos;  
uniform vec3 sunColour;  

uniform vec3 light1Pos;
uniform vec3 light1Colour;

uniform vec3 viewPos;
// texture    
uniform sampler2D ourTexture;
uniform sampler2D earthNightTexture;

// switch
uniform int earthSwitch;

// declare function
vec3 calcPhong(vec3 lightColour, vec3 lightPos, float ambientStrength, float diffusionStrength, float specularStrength);

void main()
{  	 
    vec3 finalColour;
    // final calc
    if (earthSwitch == 1) 
    {
        vec3 sumPhong = calcPhong(sunColour, sunPos, 0.0, 1.0, 0.65) + calcPhong(light1Colour, light1Pos, 0.0, 1.0, 0.65);

        // night time mapping
        float diffusionStrength = 0.0;
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(sunPos - FragPos);
        float lightIntensity = max(dot(norm, lightDir), diffusionStrength);

        float blendFactor = clamp(1-lightIntensity, 0.0, 1.0);
        vec4 dayColour = texture(ourTexture, TexCoord);
        vec4 nightColour = texture(earthNightTexture, TexCoord);
        finalColour = sumPhong * mix(dayColour, nightColour, blendFactor).rgb;
    } else
    {
        vec3 sumPhong = calcPhong(sunColour, sunPos, 0.1, 0.2, 0.65) + calcPhong(light1Colour, light1Pos, 0.1, 0.2, 0.65);
        vec4 planetColour = texture(ourTexture, TexCoord);

        finalColour = sumPhong * planetColour.rgb;
    }

    FragColor = vec4(finalColour, 1.0);
}

vec3 calcPhong(vec3 lightColour, vec3 lightPos, float ambientStrength, float diffusionStrength, float specularStrength)
{
    // ambient lighting
    vec3 ambient = ambientStrength * lightColour;

    // diffusion lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), diffusionStrength);
    vec3 diffuse = diff * lightColour;

    // specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColour;

    return (ambient + diffuse + specular);
}