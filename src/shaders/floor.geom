R"zzz(#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection;
uniform mat4 view;

out vec4 normal;
out vec3 world_position;

void main() {
  int n = 0;
  vec3 a = gl_in[0].gl_Position.xyz;
  vec3 b = gl_in[1].gl_Position.xyz;
  vec3 c = gl_in[2].gl_Position.xyz;
  vec3 u = normalize(b - a);
  vec3 v = normalize(c - a);

  normal = normalize(vec4(normalize(cross(u, v)), 0.0));

  for (n = 0; n < gl_in.length(); n++) {
    world_position = gl_in[n].gl_Position.xyz;
    gl_Position = projection * view * gl_in[n].gl_Position;
    EmitVertex();
  }
  EndPrimitive();
}
)zzz"
