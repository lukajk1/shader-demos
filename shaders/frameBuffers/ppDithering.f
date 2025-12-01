#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

// Configurable constants - adjust these as needed
const float DITHER_SIZE = 2.0;      // Pixelation amount (1.0 = no pixelation)
const float BIT_DEPTH = 4.0;        // Number of gray levels (2, 4, 8, 16, etc.)
const float CONTRAST = 1.0;         // Contrast adjustment (1.0 = normal)
const float OFFSET = 0.0;           // Brightness offset (-0.5 to 0.5)

// 4x4 Bayer Matrix for dithering threshold
const mat4 BAYER_MATRIX = mat4(
    0.0,  8.0,  2.0, 10.0,
    12.0, 4.0, 14.0,  6.0,
    3.0, 11.0,  1.0,  9.0,
    15.0, 7.0, 13.0,  5.0
);

void main() 
{
    // 1. Pixelation Step
    vec2 screen_size = vec2(textureSize(screenTexture, 0)) / DITHER_SIZE;
    vec2 screen_sample_uv = floor(TexCoords * screen_size) / screen_size;
    vec3 screen_col = texture(screenTexture, screen_sample_uv).rgb;
    
    // 2. Luminosity Calculation
    float lum = dot(screen_col, vec3(0.299, 0.587, 0.114));
    
    // 3. Contrast/Offset Adjustment
    lum = (lum - 0.5 + OFFSET) * CONTRAST + 0.5;
    lum = clamp(lum, 0.0, 1.0);
    
    // 4. Determine Luminosity Bounds
    float steps = max(BIT_DEPTH - 1.0, 1.0);
    float lum_scaled_raw = lum * steps;
    
    float lum_lower = floor(lum_scaled_raw) / steps;
    float lum_upper = ceil(lum_scaled_raw) / steps;
    float lum_fraction = fract(lum_scaled_raw);
    
    // 5. Get Bayer threshold based on pixel position
    ivec2 pixelPos = ivec2(gl_FragCoord.xy);
    int x = pixelPos.x % 4;
    int y = pixelPos.y % 4;
    float threshold = BAYER_MATRIX[x][y] / 16.0; // Normalize to 0-1
    
    // Adjust threshold slightly
    threshold = threshold * 0.99 + 0.005;
    
    // 6. Dithering Decision
    float ramp_val = lum_fraction < threshold ? 0.0 : 1.0;
    
    // 7. Final Color Output
    float final_grayscale = mix(lum_lower, lum_upper, ramp_val);
    FragColor = vec4(vec3(final_grayscale), 1.0);
}