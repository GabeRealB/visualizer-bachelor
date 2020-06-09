#version 330 core

@program sampler2D 1 view

in vec2 TexCoords;
out vec4 outColor;

uniform sampler2D view;

void main() {
    outColor = texture(view, TexCoords);
}