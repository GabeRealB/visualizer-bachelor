#version 410 core

@program sampler2D 1 view_0

in vec2 TexCoords;
out vec4 outColor;

uniform sampler2D view_0;

void main() {
    outColor = texture(view_0, TexCoords);
}