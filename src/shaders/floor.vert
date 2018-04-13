R"zzz(
#version 330 core

uniform vec4 view;
uniform vec4 light_position;

in vec3 vertex_position;

out vec4 vs_light_position;

void main() {
	gl_Position = view * vec4(vertex_position, 1.0);
  vs_light_position = -gl_Position + view * light_position;
}
)zzz"
