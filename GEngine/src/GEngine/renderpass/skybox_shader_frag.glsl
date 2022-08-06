#version 410
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube cubemap_texture;

void main()
{    
    FragColor = texture(cubemap_texture, TexCoords);
}
