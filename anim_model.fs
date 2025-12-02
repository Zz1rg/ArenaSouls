#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;

// Multiple light sources
uniform vec3 sunLightDir;        // Directional sun light
uniform vec3 sunLightColor;
uniform vec3 pointLightPos;      // Point light position
uniform vec3 pointLightColor;
uniform vec3 ambientLight;

// Material properties
uniform float shininess;
uniform float specularStrength;

// Calculate directional light (sun)
vec3 CalcDirectionalLight(vec3 lightDir, vec3 lightColor, vec3 normal, vec3 viewDir)
{
    // Diffuse shading
    float diff = max(dot(normal, -lightDir), 0.0);
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(-lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    // Combine results
    vec3 diffuse = lightColor * diff;
    vec3 specular = lightColor * spec * specularStrength;
    
    return diffuse + specular;
}

// Calculate point light
vec3 CalcPointLight(vec3 lightPos, vec3 lightColor, vec3 fragPos, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(lightPos - fragPos);
    
    // Attenuation
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    
    // Combine results
    vec3 diffuse = lightColor * diff * attenuation;
    vec3 specular = lightColor * spec * specularStrength * attenuation;
    
    return diffuse + specular;
}

void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Get base color from texture
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // Start with ambient lighting
    vec3 result = ambientLight;
    
    // Add directional light (sun)
    result += CalcDirectionalLight(sunLightDir, sunLightColor, norm, viewDir);
    
    // Add point light
    result += CalcPointLight(pointLightPos, pointLightColor, FragPos, norm, viewDir);
    
    // Apply lighting to texture
    result *= texColor.rgb;
    
    // Add subtle rim lighting for depth
    float rimPower = 1.0 - max(0.0, dot(norm, viewDir));
    vec3 rimColor = vec3(0.3, 0.4, 0.6) * pow(rimPower, 3.0) * 0.5;
    result += rimColor;
    
    FragColor = vec4(result, texColor.a);
}
