R"zzz(#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection;
uniform mat4 view;

out vec3 camera_normal;
out vec3 camera_position;

void main() {
  vec3 a = gl_in[0].gl_Position.xyz;
  vec3 b = gl_in[1].gl_Position.xyz;
  vec3 c = gl_in[2].gl_Position.xyz;
  vec3 u = normalize(b - a);
  vec3 v = normalize(c - a);

  camera_normal = normalize(view * vec4(cross(u, v), 0)).xyz;

  for (int n = 0; n < gl_in.length(); n++) {
    vec4 cp = view * gl_in[n].gl_Position;
    camera_position = (cp / cp[3]).xyz;
    gl_Position = projection * cp;
    EmitVertex();
  }
  EndPrimitive();
}
)zzz"
