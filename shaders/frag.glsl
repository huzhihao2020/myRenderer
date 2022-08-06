#version 410
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D cube_texture;

void main()
{
    // FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
    FragColor = texture(cube_texture, TexCoord);
}
