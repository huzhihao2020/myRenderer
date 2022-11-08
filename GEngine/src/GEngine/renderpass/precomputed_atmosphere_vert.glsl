#version 410
layout(location = 0) in vec4 vertex;

uniform mat4 model_from_view;
uniform mat4 view_from_clip;

out vec3 view_ray;

void main() {
  view_ray = (model_from_view * vec4((view_from_clip * vertex).xyz, 0.0)).xyz;
  gl_Position = vertex;
}