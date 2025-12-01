#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
const vec2 texelSize = vec2(1.0/400.0, 1.0/300.0);

// 3x3 Gaussian Kernel Weights
// The matrix is:
// 1 2 1
// 2 4 2
// 1 2 1
// The sum of all weights is 16.
const float kernel[9] = float[](
    1.0, 2.0, 1.0,
    2.0, 4.0, 2.0,
    1.0, 2.0, 1.0
);
const float kernel_sum = 16.0;

// Texel offsets for the 3x3 neighborhood
vec2 offsets[9] = vec2[](
    vec2(-texelSize.x,  texelSize.y), // Top-Left
    vec2( 0.0f,          texelSize.y), // Top-Center
    vec2( texelSize.x,   texelSize.y), // Top-Right
    vec2(-texelSize.x,   0.0f),        // Center-Left
    vec2( 0.0f,          0.0f),        // Center-Center
    vec2( texelSize.x,   0.0f),        // Center-Right
    vec2(-texelSize.x, -texelSize.y), // Bottom-Left
    vec2( 0.0f,        -texelSize.y), // Bottom-Center
    vec2( texelSize.x, -texelSize.y)  // Bottom-Right
);

void main()
{
    vec4 finalColor = vec4(0.0);
    
    // Perform the 3x3 convolution
    for(int i = 0; i < 9; i++)
    {
        // Sample the color at the offset position
        vec4 color = texture(screenTexture, TexCoords + offsets[i]);
        
        // Multiply color by the kernel weight and accumulate
        finalColor += color * kernel[i];
    }
    
    // Normalize the final accumulated color by dividing by the sum of the kernel weights (16.0)
    FragColor = finalColor / kernel_sum;
}