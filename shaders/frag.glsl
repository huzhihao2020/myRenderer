#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;

void main()
{
    FragColor = vec4(Normal, 1.0);
    //FragColor = vec4(Normal, 1.0);
}
