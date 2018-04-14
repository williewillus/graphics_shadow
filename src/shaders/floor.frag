R"zzz(
#version 330 core

in vec4 light_direction;
in vec3 world_position;

out vec4 fragment_color;

void main() {
  vec4 gray = vec4(0.97, 0.97, 0.97, 1.0);
  vec4 white = vec4(1.0, 1.0, 1.0, 1.0);

  if (floor(mod(world_position[0], 2)) == 1) {
      if (floor(mod(world_position[2], 2)) == 0) {
          fragment_color = white;
      } else {
          fragment_color = gray;
      }
  } else {
      if (floor(mod(world_position[2], 2)) == 0) {
          fragment_color = gray;
      } else {
          fragment_color = white;
      }
  }
	// fragment_color = vec4(1.0, 0.0, 0.0, 1.0);
}
)zzz"
