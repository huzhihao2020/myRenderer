#version 410
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_base_color;
uniform sampler2D texture_roughness;
uniform sampler2D texture_metallic;
uniform sampler2D texture_ao;
uniform sampler2D texture_emissive;

#define PI 3.1415926

vec3 ToLinear(vec3 v) { return pow(v,     vec3(2.2)); }
vec3 ToSRGB(vec3 v)   { return pow(v, vec3(1.0/2.2)); }

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

vec3 CookTorranceBRDF(vec3 viewdir, vec3 normal, vec3 lightdir) {
  float NdotL = max(dot(normal, lightdir), 0.0);
  float NdotV = max(dot(normal, viewdir), 0.0);
  vec3 bisector_h = normalize(lightdir + viewdir);
  float HdotN = max(dot(bisector_h, normal), 0.01);
  float HdotV = max(dot(bisector_h, viewdir), 0.01);

  vec3 base_color = ToLinear(texture(texture_base_color, TexCoords).rgb);
  float metalness = texture(texture_metallic, TexCoords).r;
  vec3 diffuse_brdf = (1.0 - metalness) * base_color / PI;

  // specular
  float roughness = texture(roughness_texture, TexCoords).r;
  float D = DistributionGGXTR(HdotN, roughness);

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, base_color, metalness);
  vec3 F = FresnelUEApprox(HdotV, F0);
  vec3 ks = F;
  vec3 kd = vec3(1.0) - ks;

  // (a+1)^2/8 for direct lighting
  float k = (roughness + 1.0) * (roughness + 1.0) / 8.0; 
  float G = PartialGeometryGGX(NdotL, k) * PartialGeometryGGX(NdotV, k);

  float denom = 4.0 * max(NdotL * NdotV, 0.0001);
  vec3 specular_brdf = D * F * G / denom;

  vec3 brdf = (diffuse_brdf * kd  + specular_brdf);

  vec3 color = brdf * NdotL;
  
  return color;
}

void main()
{ 
    vec3 lightPos = vec3(3, 3, 3);
    CookTorranceBRDF(view, normal, light);
    FragColor = texture(texture_diffuse, TexCoords);
    // FragColor = texture(texture_normal, TexCoords);
}
