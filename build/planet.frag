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
float lightIntensity(vec3 lightColour, vec3 lightPos, float diffusionStrength);

void main()
{  	 
    vec3 finalColour;
    // final calc
    if (earthSwitch == 1) 
    {
        vec3 sumPhong = calcPhong(sunColour, sunPos, 0.1, 0.2, 0.8) + calcPhong(light1Colour, light1Pos, 0.1, 0.2, 0.8);

        // Combine light intensities
        float combinedLightIntensity = max(lightIntensity(sunColour, sunPos, 0.2), lightIntensity(light1Colour, light1Pos, 0.2));
        float blendFactor = clamp(1.0 - combinedLightIntensity, 0.0, 1.0);
        
        vec4 dayColour = texture(ourTexture, TexCoord);
        dayColour = vec4(sumPhong * dayColour.rgb, 1.0);
        vec4 nightColour = texture(earthNightTexture, TexCoord);
        // mix texture colours
        vec4 mixed_texture = mix(dayColour, nightColour, blendFactor);
        
        finalColour = mixed_texture.rgb;
        FragColor = vec4(finalColour, 1.0);
    } else
    {
        vec3 sumPhong = calcPhong(sunColour, sunPos, 0.1, 0.2, 0.8) + calcPhong(light1Colour, light1Pos, 0.1, 0.2, 0.8);
        vec4 planetColour = texture(ourTexture, TexCoord);

        finalColour = sumPhong * planetColour.rgb;
        FragColor = vec4(finalColour, planetColour.a);
    }
}

float lightIntensity(vec3 lightColour, vec3 lightPos, float diffusionStrength)
{
    if (lightColour == vec3(0.0)) return 0.0;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float lightIntensity = max(dot(norm, lightDir), diffusionStrength);
    return lightIntensity;
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
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColour;

    return (ambient + diffuse + specular);
}