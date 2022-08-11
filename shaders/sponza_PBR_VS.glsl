#version 410
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 FragPosViewspace;
    vec2 TexCoords;
    mat3 TBN;
    vec3 Normal;
    struct PointLight {
        vec3 position;
        vec3 color;
        float intensity;
        float attenuation;
    }lights[3];
}vs_out;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    mat4 view_model_transform = u_view * u_model;
    vs_out.FragPosViewspace = (view_model_transform * vec4(aPos, 1.0)).xyz;

    vec3 N = normalize(mat3(transpose(inverse(view_model_transform))) * aNormal);
    vs_out.Normal = N;
    vec3 T = normalize((mat3(view_model_transform) * aTangent).xyz);
    vec3 B = normalize(cross(N, T));
    // TBN * tangent_pace_normal = view_space_normal
    vs_out.TBN = mat3(T, B, N);
    vs_out.TexCoords = aTexCoords;

    // light_pos in view space
    // vs_out.lights[0].position = vec3( 0.0, 0.0, 10.0);
    float x = 45.0;
    vs_out.lights[0].position = vec3( -x, 0.0, 0.0);
    vs_out.lights[1].position = vec3(0.0, 0.0, 0.0);
    vs_out.lights[2].position = vec3(  x, 0.0, 0.0);
    for(int i=0; i<3; i++) {
        vs_out.lights[i].color = vec3(0.7);
        vs_out.lights[i].intensity = 1500.0;
        float radius = 200.0;
        float radius2 = radius * radius;
        vs_out.lights[i].position = (u_view * vec4(vs_out.lights[i].position, 1.0)).xyz;
        vec3 pos2frag = vs_out.FragPosViewspace - vs_out.lights[i].position;
        float distance2 = dot(pos2frag, pos2frag);
        float m = distance2/radius2;
        float h = clamp(1 - m * m, 0.0, 1.0);
        vs_out.lights[i].attenuation = h*h / (distance2 + 1.0);
    }
    // gl_Position = projection_view_model * vec4(aPos, 1.0);
    gl_Position = u_projection * view_model_transform * vec4(aPos, 1.0);
}
