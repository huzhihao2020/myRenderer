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
        float intensity;
        float attenuation;
    }lights[3];
}fs_in;

uniform sampler2D texture_diffuse;
uniform bool has_diffuse_texture;
uniform bool u_linear_diffuse;
uniform sampler2D texture_base_color;
uniform bool has_base_color_texture;
uniform sampler2D texture_normal;
uniform bool has_normal_texture;
uniform bool u_normal_map_flip_green_channel;
uniform sampler2D texture_roughness;
uniform bool has_roughness_texture;
uniform sampler2D texture_metallic;
uniform bool has_metallic_texture;
uniform sampler2D texture_ao;
uniform bool has_ao_texture;
uniform sampler2D texture_emissive;
uniform bool has_alpha_texture;
uniform sampler2D texture_alpha;

uniform sampler2D texture_metallic_roughness;

uniform float u_metallic;
uniform float u_roughness;
uniform vec3 u_basecolor;

uniform vec2 u_viewport_size;

#define PI 3.1415926

struct FragAttribute {
  vec3 base_color;
  vec3 normal;
  float roughness;
  float metalness;
  float ao;
  vec3 emissive;
}frag_attribute;

vec3 ToLinear(vec3 v) { return pow(v,     vec3(2.2)); }
vec3 ToSRGB(vec3 v)   { return pow(v, vec3(1.0/2.2)); }

float near = 0.1; 
float far  = 300.0; 
float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

// A filmic tone mapping curve, default tone mapping method of UE 4.8
vec3 ACESToneMapping(vec3 color, float adapted_lum) {
  // adapted_lum is computed according to the luminance of the entire image
  // https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
  const float A = 2.51;
  const float B = 0.03;
  const float C = 2.43;
  const float D = 0.59;
  const float E = 0.14;
  color *= adapted_lum;
  return (color * (A * color + B)) / (color * (C * color + D) + E);
}

// Reinhard tone mapping
vec3 ReinhardToneMapping(vec3 color, float adapted_lum) {
  const float middle_grey = 1.0;
  color *= middle_grey / adapted_lum;
  return color / (vec3(1.0) + color);
}

// D
float DistributionGGXTR(float NdotH, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float m = (NdotH * NdotH * (a2 - 1.0) + 1.0);
  float denom = PI * m * m;
  return a2 / denom;
}

// F
vec3 FresnelSchlick(float HdotV, vec3 F0) {
  float m = clamp(1.0 - HdotV, 0.0, 1.0);
  float m2 = m * m;
  return F0 + (vec3(1.0) - F0) * m2 * m2 * m;
}

// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
vec3 FresnelUEApprox(float HdotV, vec3 F0) {
  return F0 + (vec3(1.0)-F0) * exp2((((-5.55473*HdotV)-6.98316)*HdotV));
}

// G = G1 * G2
float PartialGeometryGGX(float dot, float k) {
  float denom = dot * (1.0 - k) + k;
  return dot / denom;
}

// GDC 2017 Hammon
float GeometryHammon(float NdotV, float NdotL, float a) {
  float nom = 2.0 * NdotV * NdotL;
  float denom = NdotL*(NdotV*(1.0-a)+a) + NdotV*(NdotL*(1.0-a)+a);
  return nom / denom;
}

// TODO: 1. replace ambient lighting with IBL (environment lighting)
//       2. improve tone mapping method for hdr effect

vec3 CookTorranceBRDF(FragAttribute frag_attribute, vec3 viewdir, vec3 lightdir, vec3 radiance) {
  float NdotL = max(dot(frag_attribute.normal, lightdir), 0.0);
  float NdotV = max(dot(frag_attribute.normal, viewdir), 0.0);
  vec3 bisector_h = normalize(lightdir + viewdir);
  float HdotN = max(dot(bisector_h, frag_attribute.normal), 0.01);
  float HdotV = max(dot(bisector_h, viewdir), 0.01);

  vec3 diffuse_brdf = (1.0 - frag_attribute.metalness) * frag_attribute.base_color / PI;

  // specular
  float D = DistributionGGXTR(HdotN, frag_attribute.roughness);

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, frag_attribute.base_color, frag_attribute.metalness);
  // vec3 F = FresnelUEApprox(HdotV, F0);
  vec3 F = FresnelSchlick(HdotV, F0);
  vec3 ks = F;
  vec3 kd = vec3(1.0) - ks;

  // (a+1)^2/8 for direct lighting
  float k = (frag_attribute.roughness + 1.0) * (frag_attribute.roughness + 1.0) / 8.0; 
  float G = PartialGeometryGGX(NdotL, k) * PartialGeometryGGX(NdotV, k);

  float denom = 4.0 * max(NdotL * NdotV, 0.0001);
  vec3 specular_brdf = D * F * G / denom;

  vec3 brdf = (diffuse_brdf * kd  + specular_brdf);

  vec3 color = brdf * NdotL * radiance;
  
  return color;
}

void main()
{ 
  frag_attribute.normal = normalize(fs_in.Normal); 
  // it seems something wrong with the normal map of sponza.obj
  if(has_normal_texture) {
      frag_attribute.normal = texture(texture_normal, fs_in.TexCoords).rgb;
      frag_attribute.normal = frag_attribute.normal * 2.0 - vec3(1.0);
      // frag_attribute.normal = normalize(frag_attribute.normal);
      frag_attribute.normal = normalize(fs_in.TBN * frag_attribute.normal);
  }
  vec4 diiffuse_rgba = texture(texture_base_color, fs_in.TexCoords);
  if(diiffuse_rgba.a < 0.5) {
    discard;
  }
  frag_attribute.base_color = ToLinear(diiffuse_rgba.rgb);
  // if(has_base_color_texture) {
  //   frag_attribute.base_color = ToLinear(texture(texture_base_color, fs_in.TexCoords).rgb);
  // }
  // if(has_diffuse_texture) {
  //     frag_attribute.base_color = ToLinear(texture(texture_diffuse, fs_in.TexCoords).rgb);
  // }
  
  frag_attribute.ao = 1.0;
  if(has_ao_texture) {
    frag_attribute.ao = texture(texture_ao, fs_in.TexCoords).r;
  }

  vec3 metalness_roughness = texture(texture_metallic_roughness, fs_in.TexCoords).rgb;

  frag_attribute.roughness = metalness_roughness.g;
  frag_attribute.metalness = metalness_roughness.b;
  
  vec3 Lo = vec3(0.0);
  for(int i=0; i<3; i++) {
    vec3 light_dir = normalize(fs_in.lights[i].position - fs_in.FragPosViewspace);
    vec3 view_dir = normalize(-fs_in.FragPosViewspace);
    vec3 radiance = fs_in.lights[i].color * fs_in.lights[i].attenuation * fs_in.lights[i].intensity;
    Lo += CookTorranceBRDF(frag_attribute, view_dir, light_dir, radiance);
  }
  vec3 ambient = vec3(0.03) * frag_attribute.base_color;
  Lo += ambient * frag_attribute.ao;
  Lo = ACESToneMapping(Lo, 1.0);
  // Lo = ReinhardToneMapping(Lo, 1.0);
  Lo = ToSRGB(Lo);

  if(gl_FragCoord.x < 0.2 * u_viewport_size[0]) {
    FragColor = vec4(Lo, 1.0);
  }
  else if(gl_FragCoord.x < 0.4 * u_viewport_size[0]) {
    FragColor = vec4(vec3(frag_attribute.base_color), 1.0);
  }
  else if(gl_FragCoord.x < 0.6 * u_viewport_size[0]) {
    FragColor = vec4(frag_attribute.normal, 1.0);
  }
  else if(gl_FragCoord.x < 0.8 * u_viewport_size[0]) {
    FragColor = vec4(vec3(metalness_roughness), 1.0);
  }
  else {
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    FragColor = vec4(vec3(depth), 1.0);
  }
  // FragColor = vec4(vec3(frag_attribute.metalness), 1.0); // no color
}