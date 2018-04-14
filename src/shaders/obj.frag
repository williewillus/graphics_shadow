R"zzz(
#version 330 core

in vec4 light_direction;
in vec3 world_position;

out vec4 fragment_color;

void main() {
  vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);
	fragment_color = blue;
}
)zzz"
