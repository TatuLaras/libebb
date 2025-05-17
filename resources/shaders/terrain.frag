#version 330

#define TERRAIN_MAX_TEXTURES 10

in vec2 fragTexCoord;
in float texture_selection[10];
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D textures[TERRAIN_MAX_TEXTURES];
uniform vec4 colDiffuse;

void main()
{
    vec4 texel_color = vec4(vec3(0.0), 1.0);

    for (int i = 0; i < TERRAIN_MAX_TEXTURES; i++) {
        vec4 current_texel_color = texture(textures[i], fragTexCoord);
        texel_color = mix(texel_color, current_texel_color, texture_selection[i]);
    }

    finalColor = texel_color * colDiffuse * fragColor;
}
