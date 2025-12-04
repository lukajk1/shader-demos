#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;

void main()
{   
    FragColor = vec4(1., 1., 1., 1.);
}