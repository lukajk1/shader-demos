#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;

uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{   
    FragColor = vec4(lightColor, 1.);
}