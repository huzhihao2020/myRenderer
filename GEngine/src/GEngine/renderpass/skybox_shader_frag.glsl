#version 410
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube cubemap_texture;

void main()
{    
    FragColor = textureLod(cubemap_texture, TexCoords, 0.0);
    // FragColor = texture(cubemap_texture, TexCoords);
}
