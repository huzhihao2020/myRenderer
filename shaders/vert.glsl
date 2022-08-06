#version 410
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 Normal;
out vec2 TexCoord;

uniform mat4 projection_view_model;

void main()
{
    gl_Position = projection_view_model * vec4(aPos, 1.0);
    Normal = aNormal;
    TexCoord = aTexCoord;
}
