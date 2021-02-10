#version 410 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;

void main() {
    gl_Position = vec4(a_position, 0.0f, 1.0f);
}

