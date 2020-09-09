#version 410 core

@program mat4x4 1 modelMatrix
@program mat4x4 1 viewProjectionMatrix

uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 texCoords;

flat out int Side;
out vec2 TexCoords;

void main () {
    Side = int(texCoords.w);
    TexCoords = texCoords.xy;
    gl_Position = viewProjectionMatrix * modelMatrix * pos;
}