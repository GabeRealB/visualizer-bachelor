#version 330 core

@program mat4x4 1 transMatrix

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 texCoords;

out vec2 TexCoords;

uniform mat4 transMatrix;

void main() {
    gl_Position = transMatrix * vec4(pos.x, pos.y, 0, 1);
    TexCoords = texCoords.xy;
}

