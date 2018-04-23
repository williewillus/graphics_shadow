R"zzz(
#version 330 core

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 projection;
uniform mat4 view;
uniform vec4 light_pos;

const int front_vertices[3] = int[3](0, 2, 4);
const int back_vertices[3] = int[3](0, 4, 2);
float EPSILON = 0.01;

void quad(vec4 a, vec4 b) {
  vec4 light_a_dir = a - light_pos / light_pos.w;
  gl_Position = projection * view * (a + EPSILON * normalize(light_a_dir));
  EmitVertex();

  gl_Position = projection * view * light_a_dir;
  EmitVertex();

  vec4 light_b_dir = b - light_pos / light_pos.w;
  gl_Position = projection * view * (b + EPSILON * normalize(light_b_dir));
  EmitVertex();

  gl_Position = projection * view * light_b_dir;
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
      quad(gl_in[0].gl_Position, gl_in[2].gl_Position);
    }

    n = cross(e4, e5);
    light_dir = (light_pos - gl_in[2].gl_Position).xyz;
    if (dot(n, light_dir) <= 0) {
      quad(gl_in[2].gl_Position, gl_in[4].gl_Position);
    }

    n = cross(e2, e6);
    light_dir = (light_pos - gl_in[4].gl_Position).xyz;
    if (dot(n, light_dir) <= 0) {
      quad(gl_in[4].gl_Position, gl_in[0].gl_Position);
    }

    // emit all light-facing triangles as front of volume
    for (int i = 0; i < 3; i++) {
      int vertex = front_vertices[i];

      vec4 light_dir = gl_in[vertex].gl_Position - light_pos / light_pos.w;
      gl_Position = projection * view * (gl_in[vertex].gl_Position + EPSILON * normalize(light_dir));
      EmitVertex();
    }
    EndPrimitive();
    
    // project light-facing triangles as back
    for (int i = 0; i < 3; i++) {
      int vertex = back_vertices[i];

      vec4 light_dir = gl_in[vertex].gl_Position - light_pos / light_pos.w;
      gl_Position = projection * view * light_dir;
      EmitVertex();
    }
    EndPrimitive();
  }
}

)zzz"
