R"zzz(#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection;
uniform mat4 depthMVP;

in vec4 vs_light_direction[];
in vec3 vs_world_position[];

out vec4 light_direction;
out vec3 world_position;
out vec4 shadow_coord;

void main() {
  int n = 0;
  vec3 a = gl_in[0].gl_Position.xyz;
  vec3 b = gl_in[1].gl_Position.xyz;
  vec3 c = gl_in[2].gl_Position.xyz;
  vec3 u = normalize(b - a);
  vec3 v = normalize(c - a);

  for (n = 0; n < gl_in.length(); n++) {
    world_position = vs_world_position[n];
    light_direction = normalize(vs_light_direction[n]);
    gl_Position = projection * gl_in[n].gl_Position;
    shadow_coord = depthMVP * vec4(vs_world_position[n], 1);
    EmitVertex();
  }
  EndPrimitive();
}
)zzz"
