#version 410 core

@material vec2 3 gridScale
@program mat4x4 1 modelMatrix
@program mat4x4 1 viewProjectionMatrix

uniform vec2[3] gridScale;
uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 texCoords;

out vec2 TexCoords;

void main () {;
    TexCoords = gridScale[int(texCoords.w)] * texCoords.xy;
    gl_Position = viewProjectionMatrix * modelMatrix * pos;
}