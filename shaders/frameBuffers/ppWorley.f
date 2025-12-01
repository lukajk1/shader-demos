#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// The image to be crystallized (required for filtering the image)
uniform sampler2D screenTexture; 
// Uniform to control the scale/density of the noise cells
const float scale = 70.0; // Increased scale for finer crystals
uniform float time = 0.0; // Optional for animation

// Hashing function to generate deterministic pseudo-random 2D vectors (offsets)
vec2 hash22(vec2 p)
{
    p = fract(p * mat2(127.1, 311.7, 269.5, 183.3));
	p += dot(p, p + 19.1);
    return fract(p);
}

void main()
{
    // 1. Scale coordinates
    vec2 p = TexCoords * scale; // p is in scaled coordinate space (e.g., 0 to 20)

    // 2. Find the integer coordinates of the current cell
    vec2 cell = floor(p);
    
    // 3. Initialize minimum distance and the feature point coordinates
    float minDist = 1e10; // F1 distance
    vec2 closestFeaturePoint_p = vec2(0.0); // Location of the feature point in scaled space

    // 4. Iterate over the 9 surrounding cells (3x3 grid)
    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            vec2 neighbor_cell = cell + vec2(float(i), float(j));

            // Feature point location (in local cell space [0, 1])
            vec2 feature_offset = hash22(neighbor_cell);
            
            // Feature point location in the scaled coordinate space (p)
            vec2 feature_position_p = neighbor_cell + feature_offset;
            
            // Vector from fragment (p) to the feature point
            vec2 vector_to_feature = feature_position_p - p;
            
            float dist = length(vector_to_feature);

            // Track the shortest distance (F1) and the feature point that generated it
            if (dist < minDist)
            {
                minDist = dist;
                closestFeaturePoint_p = feature_position_p;
            }
        }
    }
    
    // --- 5. Image Crystallization Sampling ---
    // a. Convert the closest feature point location back to normalized UV coordinates (0.0 to 1.0)
    vec2 sampleUV = closestFeaturePoint_p / scale;
    
    // b. Sample the original image at this calculated point
    vec4 finalColor = texture(screenTexture, sampleUV);

    // Optional: Highlight cell edges by blending with distance
    // float edge = smoothstep(0.0, 0.05, minDist); 
    // finalColor.rgb = mix(vec3(0.0), finalColor.rgb, edge); // Darken the color near the boundary

    FragColor = finalColor;
}