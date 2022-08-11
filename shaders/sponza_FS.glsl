#version 410
out vec4 FragColor;

in VS_OUT {
    vec3 FragPosViewspace;
    vec2 TexCoords;
    mat3 TBN;
    vec3 Normal;
    struct PointLight {
        vec3 position;
        vec3 color;
        float attenuation;
    }lights[3];
}fs_in;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_alpha;
uniform bool has_normal_texture;
uniform bool has_alpha_texture;

void main()
{
    if(has_alpha_texture) {
        float alpha = texture(texture_alpha, fs_in.TexCoords).r;
        if(alpha<0.9) discard;
    }
    vec3 basecolor = texture(texture_diffuse, fs_in.TexCoords).rgb;
    vec3 normal = fs_in.Normal; 
    if(has_normal_texture) {
        normal = texture(texture_normal, fs_in.TexCoords).rgb;
        normal = normal * 2.0 - vec3(1.0);
        normal = normalize(fs_in.TBN * normal);
    }
    vec3 lighting = vec3(0.0);
    // calculate light in view space
    for(int i=1; i<2; i++) {
        vec3 light_dir = normalize(fs_in.lights[i].position - fs_in.FragPosViewspace);
        vec3 view_dir = normalize(-fs_in.FragPosViewspace);
        // ambient
        vec3 ambient, diffuse, specular;
        ambient = 0.1 * basecolor;
        // diffuse
        diffuse = 0.8 * fs_in.lights[i].color * basecolor * max(dot(light_dir, normal), 0.0f);
        // specular
        vec3 half_vec = normalize(light_dir + view_dir);
        specular = 0.5 * fs_in.lights[i].color * pow(max(dot(half_vec, normal), 0.0f), 64);
        // accumulate
        lighting = lighting + (ambient + diffuse + specular) * fs_in.lights[i].attenuation;
    }
    FragColor = vec4(lighting, 1.0);
    // FragColor = vec4(normal, 1.0);
    // FragColor = texture(texture_diffuse, fs_in.TexCoords);
}
