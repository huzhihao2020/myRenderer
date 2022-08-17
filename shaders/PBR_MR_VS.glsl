#version 410
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

#define LIGHTS_NUM 4

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
// uniform lights
uniform vec3 u_light0_pos;
uniform vec3 u_light0_color;
uniform float u_light0_intensity;
uniform vec3 u_light1_pos;
uniform vec3 u_light1_color;
uniform float u_light1_intensity;
uniform vec3 u_light2_pos;
uniform vec3 u_light2_color;
uniform float u_light2_intensity;
uniform vec3 u_light3_pos;
uniform vec3 u_light3_color;
uniform float u_light3_intensity;

// out
out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec3 Normal;
    struct PointLight {
        vec3 position;
        vec3 color;
        float intensity;
        float attenuation;
    }lights[LIGHTS_NUM];
}vs_out;

// setup lights in world sapce
void SetupLights(vec3 frag_pos) {
    vs_out.lights[0].position = u_light0_pos;
    vs_out.lights[0].color = u_light0_color;
    vs_out.lights[0].intensity = u_light0_intensity;
    vs_out.lights[1].position = u_light1_pos;
    vs_out.lights[1].color = u_light1_color;
    vs_out.lights[1].intensity = u_light1_intensity;
    vs_out.lights[2].position = u_light2_pos;
    vs_out.lights[2].color = u_light2_color;
    vs_out.lights[2].intensity = u_light2_intensity;
    vs_out.lights[3].position = u_light3_pos;
    vs_out.lights[3].color = u_light3_color;
    vs_out.lights[3].intensity = u_light3_intensity;

    for(int i=0; i<LIGHTS_NUM; i++) {
        float radius = 35.0;
        float radius2 = radius * radius;
        vec3 pos2frag = frag_pos - vs_out.lights[i].position;
        float distance2 = dot(pos2frag, pos2frag);
        float m = distance2/radius2;
        float h = clamp(1 - m * m, 0.0, 1.0);
        vs_out.lights[i].attenuation = h*h / (distance2 + 1.0);
    }
}

void main()
{
    mat4 projection_view_transform = u_projection * u_view;
    vs_out.FragPos = vec3(u_model * vec4(aPos, 1.0));

    SetupLights(vs_out.FragPos);

    vec3 N = normalize(mat3(transpose(inverse(u_model))) * aNormal);
    vec3 T = normalize((mat3(u_model) * aTangent).xyz);
    vec3 B = normalize(cross(N, T));
    vs_out.TBN = mat3(T, B, N);
    vs_out.Normal = N;
    vs_out.TexCoords = aTexCoords;

    gl_Position = projection_view_transform * vec4(vs_out.FragPos, 1.0);
}
