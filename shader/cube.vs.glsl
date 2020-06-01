#version 330 core

@program mat4x4 1 modelMatrix
@program mat4x4 1 viewProjectionMatrix

uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;

layout(location = 0) in vec4 pos;

void main () {
    gl_Position = viewProjectionMatrix * modelMatrix * pos;
}