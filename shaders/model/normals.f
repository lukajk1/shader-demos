#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;

void main()
{   
	vec3 norm = normalize(Normal);

	// Map from [-1, 1] to [0, 1] for color display
	vec3 color = (norm + 1.0) / 2.0;
    
	FragColor = vec4(Normal, 1.);
}