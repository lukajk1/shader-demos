#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;

void main()
{   
    // 1. Prepare vectors
    // Normalize the Normal (input Normal might not be normalized)
    vec3 N = normalize(Normal);
    
    // Calculate world position (assuming this shader is part of a pipeline where Normal is correctly interpolated)
    // For simplicity, we'll assume the fragment's world position is needed to calculate V.
    // Since worldPos isn't an input, we use a common fallback approach: 
    // If the Normal is interpolated, we calculate V using a normalized vector to the viewer.
    // Note: In a full implementation, you'd pass the fragment's world position (fragPos) as an 'in' variable.
    // Since we don't have fragPos, we rely purely on the Normal and viewPos.
    
    // Calculate View Direction (V): from fragment to camera
    // We assume the fragment position is (0, 0, 0) relative to the Normal calculation space for simplicity.
    // A more correct way is V = normalize(viewPos - fragPos).
    // For this example, let's assume 'Normal' is in World Space, and V is calculated from a common origin.
    vec3 V = normalize(viewPos); // Simplified V (less accurate without fragPos)
    // *** A more robust approach requires 'in vec3 fragPos;' and V = normalize(viewPos - fragPos); ***
    
    // Let's use the reflection vector approximation for the view direction V, 
    // which simplifies the math and is often sufficient for a stylized Fresnel.
    // However, the standard implementation is V.N. We will proceed with V.N.

    // 2. Calculate the cosine of the angle between V and N
    // Use the absolute value since the vector V should be pointing *away* from the surface.
    // V is defined here as pointing *from* the eye *to* the surface.
    // When V and N are perpendicular, the dot product is 0.
    float cosTheta = dot(V, N); 
    // Clamp to ensure it is non-negative (sometimes necessary due to interpolation)
    cosTheta = max(0.0, abs(cosTheta)); 
    
    // 3. Apply the Schlick's Fresnel Approximation
    // R0: The base reflectivity (or F0). Typically set low for non-metals (e.g., 0.04)
    float R0 = 0.04; 
    
    // Fresnel Term (F) = R0 + (1 - R0) * (1 - cosTheta)^5
    float fresnel = R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5.0);

    // 4. Combine with Texture
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    // Use the Fresnel term to add reflectivity to the final color.
    // We mix the original texture color with a reflection color (e.g., white or a subtle highlight color)
    // The 'fresnel' value determines the blend weight.
    vec3 finalColor = texColor.rgb;
    vec3 reflectionColor = vec3(1.0, 1.0, 1.0); // White reflection
    
    // Add the Fresnel effect as an additive or blended highlight
    finalColor = mix(finalColor, reflectionColor, fresnel);
    
    FragColor = vec4(finalColor, texColor.a);
}