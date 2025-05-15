#version 330 core

layout (location = 0) in vec2 aSample;

uniform int uNSamples;
uniform int uPos;

void main()
{
   gl_Position = vec4((gl_VertexID - uPos) / float(uNSamples) * 2.0 - 1.0, aSample.x / 2.0 + 0.5, 0.0, 1.0);
}
