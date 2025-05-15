#version 330 core

layout (location = 0) in float aSample;

uniform int uNSamples;
uniform float uDftmax;

void main()
{
    bool mirrored = true;

    if (mirrored) {
        gl_Position = vec4((gl_VertexID + uNSamples / 2) % uNSamples / float(uNSamples) * 2.0 - 1.0, aSample / uDftmax / 2.0 - 0.5, 0.0, 1.0);
    } else {
        gl_Position = vec4(gl_VertexID / float(uNSamples) * 2.0 - 1.0, aSample / uDftmax / 2.0 - 0.5, 0.0, 1.0);
    }
}
