#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
    // --- 1. Define the Sobel Kernels ---
    // Horizontal Kernel (Gx) weights
    float kernel_x[9] = float[](
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    );
    
    // Vertical Kernel (Gy) weights
    float kernel_y[9] = float[](
        -1.0, -2.0, -1.0,
         0.0,  0.0,  0.0,
         1.0,  2.0,  1.0
    );

    // --- 2. Define the 9 Texel Offsets ---
    // Uses the uniform texelSize to calculate the offset relative to the texture's dimensions
    vec2 offsets[9] = vec2[](
        vec2(-texelSize.x,  texelSize.y),  // Top-Left
        vec2( 0.0f,          texelSize.y),  // Top-Center
        vec2( texelSize.x,   texelSize.y),  // Top-Right
        vec2(-texelSize.x,   0.0f),         // Center-Left
        vec2( 0.0f,          0.0f),         // Center-Center (Unused in Sobel, but kept for full 3x3 loop)
        vec2( texelSize.x,   0.0f),         // Center-Right
        vec2(-texelSize.x, -texelSize.y),  // Bottom-Left
        vec2( 0.0f,        -texelSize.y),  // Bottom-Center
        vec2( texelSize.x, -texelSize.y)   // Bottom-Right
    );

    // --- 3. Accumulate Gradients (Sx and Sy) ---
    float S_x = 0.0;
    float S_y = 0.0;

    for(int i = 0; i < 9; i++)
    {
        // Sample the color at the offset position
        vec3 colorSample = texture(screenTexture, TexCoords + offsets[i]).rgb;
        
        // Convert sample to grayscale luminance (approximated using the red channel for speed)
        // A more accurate luminance: float lum = dot(colorSample, vec3(0.2126, 0.7152, 0.0722));
        float lum = colorSample.r; 
        
        // Apply the kernel weights to the luminance and accumulate the horizontal and vertical gradients
        S_x += lum * kernel_x[i];
        S_y += lum * kernel_y[i];
    }
    
    // --- 4. Compute Final Edge Magnitude ---
    // The magnitude is the length of the resulting 2D gradient vector (Sx, Sy).
    // This value represents the strength of the edge at the current pixel.
    float magnitude = sqrt(S_x * S_x + S_y * S_y);
    
    // Clamp the magnitude to the [0.0, 1.0] range
    float edgeStrength = clamp(magnitude, 0.0, 1.0);
    
    // Output the edge strength as a grayscale color (or invert for black edges on white background)
    FragColor = vec4(vec3(edgeStrength), 1.0);
}