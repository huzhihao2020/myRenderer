#version 410
out vec4 FragColor;

#define IrradianceOnly false
#define PI 3.1415926
#define LIGHTS_NUM 4

uniform float u_metallic;
uniform float u_roughness;
uniform vec3 u_basecolor;
uniform vec3 u_view_pos;

uniform samplerCube irradiance_cubemap;
uniform samplerCube prefilter_cubemap;
uniform sampler2D brdf_lut;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
    vec3 Normal;
    struct PointLight {
        vec3 position;
        vec3 color;
        float intensity;
        float attenuation;
    }lights[LIGHTS_NUM];
}fs_in;

struct FragAttribute {
  vec3 base_color;
  vec3 normal;
  float roughness;
  float metalness;
  float ao;
}frag_attribute;

struct EnvLight{
  vec3 F0;
  vec2 brdf;
}env_ibl;

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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
  env_ibl.F0 = F0;
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

  frag_attribute.base_color = u_basecolor;  
  frag_attribute.metalness  = u_metallic;
  frag_attribute.roughness  = u_roughness;

  vec3 N = normalize(fs_in.Normal);
  vec3 V = normalize(u_view_pos - fs_in.FragPos);
  vec3 R = reflect(-V, N); 

  vec3 Lo = vec3(0.0);
  for(int i=0; i<4; i++) {
    vec3 light_dir = normalize(fs_in.lights[i].position - fs_in.FragPos);
    vec3 view_dir = V;
    vec3 radiance = fs_in.lights[i].color * fs_in.lights[i].attenuation * fs_in.lights[i].intensity;
    Lo += CookTorranceBRDF(frag_attribute, view_dir, light_dir, radiance);
  }
  if(IrradianceOnly) {
    Lo = vec3(0.0);
  }
  // irradiance color
  vec4 ambient_irradiance = texture(irradiance_cubemap, fs_in.Normal);
  vec3 ibl_diffuse = ambient_irradiance.rgb * frag_attribute.base_color;

  vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), env_ibl.F0, frag_attribute.roughness);
  vec3 kS = F;
  vec3 kD = (vec3(1.0) - kS) * (1.0 - frag_attribute.metalness);

  // ibl specular
  const float MAX_REFLECTION_LOD = 8.0;
  vec4 prefilteredColor = textureLod(prefilter_cubemap, R, MAX_REFLECTION_LOD * frag_attribute.roughness);    
  env_ibl.brdf  = texture(brdf_lut, vec2(max(dot(N, V), 0.0), frag_attribute.roughness)).rg;
  vec3 ibl_specular = prefilteredColor.rgb * (F * env_ibl.brdf.x + env_ibl.brdf.y);

  vec3 ambient = kD * ibl_diffuse + ibl_specular;

  Lo += ambient;
  // tone mapping
  Lo = ACESToneMapping(Lo, 1.0);
  // Lo = ReinhardToneMapping(Lo, 1.0);

  Lo = ToSRGB(Lo);
  // FragColor = vec4(vec3(fs_in.lights[3].color), 1.0);
  // FragColor = vec4(ambient, 1.0);
  FragColor = vec4(Lo, 1.0);
  // FragColor = prefilteredColor;
  // FragColor = ambient_irradiance;
}