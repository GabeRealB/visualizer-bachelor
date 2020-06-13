#version 330 core

@material vec4 1 diffuseColor
@material sampler2D 1 gridTexture

uniform vec4 diffuseColor;
uniform sampler2D gridTexture;

in vec2 TexCoords;
out vec4 outColor;

void main() {
    outColor = texture(gridTexture, TexCoords) * diffuseColor;
}