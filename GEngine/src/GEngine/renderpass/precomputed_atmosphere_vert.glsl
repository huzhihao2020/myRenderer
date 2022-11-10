#version 410
layout(location = 0) in vec4 vertex;

uniform mat4 model_from_view;
uniform mat4 view_from_clip;

out vec3 view_ray;
out vec2 screen_quad_texcoord;

void main() {
  screen_quad_texcoord = (vertex.xy + vec2(1.0)) / 2.0;
  view_ray = (model_from_view * vec4((view_from_clip * vertex).xyz, 0.0)).xyz;
  gl_Position = vertex;
}