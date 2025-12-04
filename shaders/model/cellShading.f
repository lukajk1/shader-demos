#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

// Point Light Constants (defined at top of file)
const vec3 LIGHT_AMBIENT = vec3(0.2, 0.2, 0.2);
const vec3 LIGHT_DIFFUSE = vec3(0.8, 0.8, 0.8);
const vec3 LIGHT_SPECULAR = vec3(1.0, 1.0, 1.0);
const float LIGHT_CONSTANT = 1.0;
const float LIGHT_LINEAR = 0.09;
const float LIGHT_QUADRATIC = 0.032;

// Material Constants
const float MATERIAL_SHININESS = 32.0;

uniform vec3 localColor;
uniform vec3 lightColor; 

// Cell Shading Constants
const int SHADING_LEVELS = 3; // Number of discrete shading levels
const float SPECULAR_THRESHOLD = 0.8; // Threshold for specular highlight
const float EDGE_THRESHOLD = 0.2; // Threshold for edge detection

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};
uniform Material material;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
uniform Light light;

void main()
{

    // --- 1. Attenuation calculation ---
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                          light.quadratic * (distance * distance));

    // --- 2. Lighting components ---
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // --- 3. Cell Shading (Toon Shading) ---

    // Diffuse component with discrete levels
    float diffuseIntensity = max(dot(norm, lightDir), 0.0);

    // Quantize diffuse to discrete levels
    float levelSize = 1.0 / float(SHADING_LEVELS);
    float level = floor(diffuseIntensity / levelSize);
    float cellDiffuse = level * levelSize;

    vec3 ambient = light.ambient * localColor * lightColor;
    vec3 diffuse = light.diffuse * cellDiffuse * localColor * lightColor;

    // Specular component (sharp highlight)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularIntensity = pow(max(dot(norm, halfwayDir), 0.0), MATERIAL_SHININESS);

    // Binary specular highlight (either on or off)
    float cellSpecular = step(SPECULAR_THRESHOLD, specularIntensity);
    vec3 specular = light.specular * cellSpecular * lightColor;

    // --- 4. Edge Detection (Rim Lighting for outlines) ---
    float rimDot = 1.0 - max(dot(viewDir, norm), 0.0);
    float rimIntensity = smoothstep(1.0 - EDGE_THRESHOLD, 1.0, rimDot);
    vec3 rimColor = vec3(0.0, 0.0, 0.0) * rimIntensity; // Black rim/outline

    // --- 5. Apply attenuation and combine results ---
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular + rimColor;

    // Output the final color
    FragColor = vec4(result, 1.0);
}
