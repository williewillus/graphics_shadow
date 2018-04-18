R"zzz(
#version 330 core

layout (triangles_adjacency) in;
layout (line_strip, max_vertices=6) out;

uniform mat4 projection;
uniform mat4 view;
uniform vec4 light_pos;

void line(int from, int to) {
  gl_Position = projection * view * gl_in[from].gl_Position;
  EmitVertex();

  gl_Position = projection * view * gl_in[to].gl_Position;
  EmitVertex();

  EndPrimitive();
}

void main() {
  vec3 e1 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 e2 = gl_in[4].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 e3 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 e4 = gl_in[3].gl_Position.xyz - gl_in[2].gl_Position.xyz;
  vec3 e5 = gl_in[4].gl_Position.xyz - gl_in[2].gl_Position.xyz;
  vec3 e6 = gl_in[5].gl_Position.xyz - gl_in[0].gl_Position.xyz;

  vec3 n = cross(e1, e2);
  vec3 light_dir = (light_pos - gl_in[0].gl_Position).xyz;
  if (dot(n, light_dir) > 0.00001) {
    n = cross(e3, e1);
    if (dot(n, light_dir) <= 0) {
      line(0, 2);
    }

    n = cross(e4, e5);
    light_dir = (light_pos - gl_in[2].gl_Position).xyz;
    if (dot(n, light_dir) <= 0) {
      line(2, 4);
    }

    n = cross(e2, e6);
    light_dir = (light_pos - gl_in[4].gl_Position).xyz;
    if (dot(n, light_dir) <= 0) {
        line(4, 0);
    }
  }
}

)zzz"
