#version 330 core

@material vec4 1 diffuseColor

uniform vec4 diffuseColor;

out vec4 outColor;

void main() {
    outColor = diffuseColor;
}