R"zzz(
#version 330 core

uniform mat4 view;
uniform vec4 light_pos;

in vec3 vertex_position;
out vec4 vs_light_direction;

void main() {
	gl_Position = view * vec4(vertex_position, 1.0);
  vs_light_direction = -gl_Position + view * light_pos;
}
)zzz"
