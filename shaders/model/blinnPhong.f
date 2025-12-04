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
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                          light.quadratic * (distance * distance));

    // --- 2. Lighting components ---
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);

    // Ambient (use simple color since we don't have texture)
    vec3 ambient = light.ambient * vec3(0.5);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(0.5);

    // Specular (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir); // Halfway vector between light and view
    float spec = pow(max(dot(norm, halfwayDir), 0.0), MATERIAL_SHININESS);
    vec3 specular = light.specular * spec * vec3(0.3);

    // --- 3. Apply attenuation and combine results ---
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;

    // Output the final color
    FragColor = vec4(result, 1.0);
}