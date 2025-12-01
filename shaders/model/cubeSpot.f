#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
// Note: lightPos is used to calculate the light direction (FragPos to light position)

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission; // Included but not used in the lighting calculation below
    float shininess;
};
uniform Material material;

struct Light {
    vec3 position;
    vec3 direction;
    float cutOff; // Inner cone angle (cosine value)

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;

void main()
{
    // 1. Calculate essential vectors
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

    // 2. Spotlight check
    // theta is the cosine of the angle between the fragment-to-light vector (lightDir)
    // and the negative light direction (the direction the light is pointing)
    // We use -light.direction because lightDir points from the fragment *to* the light source,
    // but we want the cone angle relative to the light source's vector *away* from itself.
    float theta = dot(lightDir, normalize(-light.direction));

    if(theta > light.cutOff)
    {
        // Fragment is inside the spotlight cone - perform full Phong lighting

        // Ambient: Light's ambient multiplied by the diffuse texture color
        vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

        // Diffuse: Light's diffuse scattered by the fragment's surface
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

        // Specular: Reflection strength
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

        // Combine all lighting components
        result = ambient + diffuse + specular;
    }
    else
    {
        // Fragment is outside the spotlight cone - use only ambient light
        result = light.ambient * vec3(texture(material.diffuse, TexCoords));
    }

    // Output the final color
    FragColor = vec4(result, 1.0);
}