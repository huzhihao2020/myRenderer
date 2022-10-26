#version 410
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;

uniform vec3 u_diffuse_color;

void main()
{
    // vec3 color = pow(u_diffuse_color, vec3(1.0/2.2));
    vec3 color = texture(texture_diffuse, TexCoords).rgb;
    FragColor = vec4(color, 1.0);
}
