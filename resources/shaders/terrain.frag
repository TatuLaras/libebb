#version 330

in vec2 fragTexCoord;
in float texture_selection[10];
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform sampler2D texture6;

vec4 texture_indexed(int i) {
    if (i == 0) return texture(texture0, fragTexCoord);
    if (i == 1) return texture(texture1, fragTexCoord);
    if (i == 2) return texture(texture2, fragTexCoord);
    if (i == 3) return texture(texture3, fragTexCoord);
    if (i == 4) return texture(texture4, fragTexCoord);
    if (i == 5) return texture(texture5, fragTexCoord);
    if (i == 6) return texture(texture6, fragTexCoord);
    return vec4(vec3(0.0), 1.0);
}

void main()
{
    vec4 texel_color = vec4(vec3(0.0), 1.0);

    for (int i = 0; i < 7; i++) {
        vec4 current_texel_color = texture_indexed(i);
        texel_color = mix(texel_color, current_texel_color, texture_selection[i]);
    }

    finalColor = texel_color * fragColor;
}
