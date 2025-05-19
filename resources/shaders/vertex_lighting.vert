#version 330

#define TERRAIN_MAX_TEXTURES 7

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
in vec3 vertexNormal;
out vec2 fragTexCoord;
out vec4 fragColor;
out float texture_selection[10];

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

#define MAX_LIGHTS 16

#define LIGHT_NULL 0
#define LIGHT_DIRECTIONAL 1
#define LIGHT_POINT 2

struct LightSource {
    int enabled;
    int type;
    float intensity;
    float intensity_cap;
    vec3 position;
    vec3 target;
    vec4 color;
};

uniform LightSource lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform int shadingDisabled;

void main()
{
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp * vec4(vertexPosition, 1.0);

    for (int i = 0; i < TERRAIN_MAX_TEXTURES; i++) {
        if (int(vertexColor.y * 255.0 + 0.01) == i)
            texture_selection[i] = 1.0;
        else
            texture_selection[i] = 0.0;
    }

    if (shadingDisabled == 1 || vertexColor.x == 0) {
        fragColor = vec4(1.0);
        return;
    }

    vec3 transformedVertexPosition = (matModel * vec4(vertexPosition, 1.0)).xyz;
    vec3 transformedVertexNormal = normalize((matNormal * vec4(vertexNormal, 1.0)).xyz);

    vec3 light = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (lights[i].type == LIGHT_NULL) break;
        if (lights[i].enabled == 0) continue;

        vec3 lightDirection = normalize(lights[i].position - transformedVertexPosition);

        float lightIntensity = max(dot(lightDirection, transformedVertexNormal), 0.0);
        float r = distance(transformedVertexPosition, lights[i].position);
        lightIntensity *= lights[i].intensity / pow(r, 2);
        lightIntensity = min(lightIntensity, lights[i].intensity_cap);

        light += vec3(lights[i].color) * lightIntensity;
    }

    light += vec3(ambient);
    fragColor = vec4(light, 1.0);
}
