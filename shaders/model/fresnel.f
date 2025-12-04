#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;
uniform vec3 localColor;

const float R0 = 0.04; // Base reflectivity for non-metals

void main()
{
    // Normalize the normal and calculate view direction
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // Calculate the Fresnel term using Schlick's approximation
    // cosTheta is the angle between view direction and normal
    float cosTheta = max(dot(V, N), 0.0);

    // Fresnel Term: F = R0 + (1 - R0) * (1 - cosTheta)^5
    float fresnel = R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5.0);

    // Mix base color with reflection color based on Fresnel term
    vec3 reflectionColor = vec3(1.0, 1.0, 1.0);
    vec3 finalColor = mix(localColor, reflectionColor, fresnel);

    FragColor = vec4(finalColor, 1.0);
}