#version 330 core

out vec4 FragColor;

uniform samplerBuffer functionSampler;
uniform vec2 uRes;
uniform int uNSamples;
uniform int uPos;

float sampleFunction(float r) {
    // Map radius [0,1] to texture coordinate [0,size-1]
    float texCoord = r * (uNSamples - 1);
    
    // Get the two nearest samples
    int index0 = int(floor(texCoord));
    int index1 = int(ceil(texCoord));
    
    // Clamp to valid range
    index0 = clamp(index0, 0, int(uNSamples)-1);
    index1 = clamp(index1, 0, int(uNSamples)-1);
    
    // Linear interpolation
    float t = fract(texCoord);
    float y0 = texelFetch(functionSampler, uPos + index0).r;
    float y1 = texelFetch(functionSampler, uPos + index1).r;
    
    return mix(y0, y1, t);
}

void main() {
    // Center coordinates [-1,1] with correct aspect ratio
    vec2 uv = (2.0 * gl_FragCoord.xy - uRes.xy) / uRes.y;
    
    // Calculate radius [0, √2] (max distance from center)
    float r = length(uv);
    
    // Normalize radius to [0,1] range
    float normalized_r = r / sqrt(2.0);
    
    // Sample the function
    float g_value = sampleFunction(normalized_r);
    
    // Level line calculation with antialiasing
    float levelSpacing = 0.2;
    float level = g_value / levelSpacing;
    float distance = fract(level);
    
    // Smooth step for antialiased lines
    float lineWidth = 0.1;
    float line = smoothstep(0.5-lineWidth, 0.5+lineWidth, distance);
    line *= smoothstep(0.5-lineWidth, 0.5+lineWidth, 1.0-distance);
    
    // Color mapping (example: thermal colors)
    vec3 color = vec3(
        smoothstep(0.0, 0.5, g_value),
        smoothstep(0.3, 0.7, g_value),
        smoothstep(0.6, 1.0, g_value)
    );
    
    // Final color with background
    FragColor = vec4(mix(vec3(0.1), color, 1.0-line), 1.0);
}
