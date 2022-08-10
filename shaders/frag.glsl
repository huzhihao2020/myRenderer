#version 410
out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D diffuse_marble;

void main()
{
    // FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
    // FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    FragColor = texture(diffuse_marble, TexCoord);
}
