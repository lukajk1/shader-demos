#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

// The radius for the filter. Kuwahara uses a 5x5 window, so radius is 2.
const int RADIUS = 2;

// Structure to hold the mean and variance for one of the four quadrants
struct Quadrant
{
    vec3 mean;
    float variance;
};

// Function to calculate the variance of a single color channel (e.g., Red) for efficiency
// Variance = Mean of Squares - Square of the Mean
float calculateVariance(vec3 colorSum, vec3 colorSqSum, float count)
{
    // The variance is calculated for the luminance (approximated by the red channel)
    float mean_r  = colorSum.r / count;
    float meanSq_r = colorSqSum.r / count;
    
    // Using a simpler 1D variance on the red channel is often sufficient and faster
    return meanSq_r - (mean_r * mean_r);
}

void main()
{
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);

    // The four quadrants are defined by their top-left corner relative to the center pixel.
    // The standard 5x5 filter uses four overlapping 3x3 regions.
    // The offset of the starting sample in TexCoords space:
    vec2 half_offset = texelSize * float(RADIUS);

    // Initialize the best result found so far
    Quadrant best = Quadrant(vec3(0.0), 10000.0); // Variance initialized to a very high number

    // Define the 4 quadrant starting points relative to the center pixel:
    // Q1: Top-Left (from -2,-2 to 0,0) - shifted back by -1,-1
    // Q2: Top-Right (from 0,-2 to 2,0) - shifted back by 1,-1
    // Q3: Bottom-Left (from -2,0 to 0,2) - shifted back by -1,1
    // Q4: Bottom-Right (from 0,0 to 2,2) - shifted back by 1,1
    
    // Loop through the 4 quadrants (represented by the sign of the starting sample position)
    for (int i = 0; i < 4; ++i)
    {
        // Calculate the starting offset for the 3x3 region
        // This setup samples an area offset by (+/-1, +/-1) from the center
        float x_sign = (i % 2 == 0) ? -1.0 : 1.0;
        float y_sign = (i < 2) ? -1.0 : 1.0;
        
        vec2 startOffset = vec2(x_sign, y_sign) * texelSize;
        
        vec3 sum = vec3(0.0);
        vec3 sum_sq = vec3(0.0);
        float count = 0.0;
        
        // Loop through the 3x3 region (from -1 to 1) relative to the startOffset
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                // Calculate the final sample coordinate
                vec2 currentOffset = startOffset + vec2(x, y) * texelSize;
                vec3 color = texture(screenTexture, TexCoords + currentOffset).rgb;
                
                // Accumulate sum and sum of squares for variance calculation
                sum += color;
                sum_sq += color * color;
                count += 1.0;
            }
        }
        
        // Calculate Mean and Variance for the current quadrant
        Quadrant current;
        current.mean = sum / count;
        current.variance = calculateVariance(sum, sum_sq, count);
        
        // Check if this quadrant has the lowest variance found so far
        if (current.variance < best.variance)
        {
            best = current;
        }
    }
    
    // The final color is the mean of the quadrant with the lowest variance
    FragColor = vec4(best.mean, 1.0);
}