#version 330

#define TERRAIN_MAX_TEXTURES 10

in vec2 fragTexCoord;
in float texture_selection[10];
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D textures[TERRAIN_MAX_TEXTURES];
uniform vec4 colDiffuse;

vec4 texture_indexed(int i) {
    if (i == 0) return texture(textures[0], fragTexCoord);
    if (i == 1) return texture(textures[1], fragTexCoord);
    if (i == 2) return texture(textures[2], fragTexCoord);
    if (i == 3) return texture(textures[3], fragTexCoord);
    if (i == 4) return texture(textures[4], fragTexCoord);
    if (i == 5) return texture(textures[5], fragTexCoord);
    if (i == 6) return texture(textures[6], fragTexCoord);
    return vec4(vec3(0.0), 1.0);
}

void main()
{
    vec4 texel_color = vec4(vec3(0.0), 1.0);

    for (int i = 0; i < TERRAIN_MAX_TEXTURES; i++) {
        vec4 current_texel_color = texture_indexed(i);
        texel_color = mix(texel_color, current_texel_color, texture_selection[i]);
    }

    finalColor = texel_color * colDiffuse * fragColor;
}
