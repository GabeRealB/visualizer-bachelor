#version 410 core

@material vec4 1 diffuseColor
@material sampler2D 1 gridTexture

uniform vec4 diffuseColor;
uniform sampler2D gridTexture;

in vec2 TexCoords;
out vec4 outColor;

void main() {
    vec4 textureColor = vec4(texture(gridTexture, TexCoords).xyz, 1.0f);
    outColor = textureColor * diffuseColor;

    if (textureColor == vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
        outColor.a = 1.0f;
    }

    if (outColor.a == 0.0f) {
        discard;
    }
}