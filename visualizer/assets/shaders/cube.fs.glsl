#version 410 core

@material vec4 1 diffuseColor
@material sampler2D 1 gridTextureFront
@material sampler2D 1 gridTextureSide
@material sampler2D 1 gridTextureTop

uniform vec4 diffuseColor;
uniform sampler2D gridTextureFront;
uniform sampler2D gridTextureSide;
uniform sampler2D gridTextureTop;

flat in int Side;
in vec2 TexCoords;
out vec4 outColor;

void main() {
    vec4 textureColor = vec4(0.0f);

    if (Side == 0) {
        textureColor = vec4(texture(gridTextureFront, TexCoords).xyz, 1.0f);
    } else if (Side == 1) {
        textureColor = vec4(texture(gridTextureTop, TexCoords).xyz, 1.0f);
    } else if (Side == 2) {
        textureColor = vec4(texture(gridTextureSide, TexCoords).xyz, 1.0f);
    }

    outColor = textureColor * diffuseColor;

    if (textureColor == vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
        outColor.a = 1.0f;
    }

    if (outColor.a == 0.0f) {
        discard;
    }
}