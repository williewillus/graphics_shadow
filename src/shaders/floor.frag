R"zzz(
#version 330 core

in vec4 light_direction;
in vec3 world_position;

out vec4 fragment_color;

void main() {
  if (floor(mod(world_position[0], 2)) == 1) {
      if (floor(mod(world_position[2], 2)) == 0) {
          fragment_color = vec4(1.0, 1.0, 1.0, 1.0);
      } else {
          fragment_color = vec4(0.95, 0.95, 0.95, 1.0);
      }
  } else {
      if (floor(mod(world_position[2], 2)) == 0) {
          fragment_color = vec4(0.95, 0.95, 0.95, 1.0);
      } else {
          fragment_color = vec4(1.0, 1.0, 1.0, 1.0);
      }
  }
	// fragment_color = vec4(1.0, 0.0, 0.0, 1.0);
}
)zzz"
