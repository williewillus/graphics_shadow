R"zzz(
#version 330 core

in vec4 light_direction;
in vec3 world_position;
in vec4 shadow_coord;

out vec4 fragment_color;

uniform sampler2D shadow_map;

bool in_shadow(vec4 coord_) {
  vec3 coord = coord_.xyz / coord_.w;
  coord = coord * 0.5 + 0.5;
  if (coord.x < 0 || coord.x > 1 || coord.y < 0 || coord.y > 1) {
    return false;
  }
  float bias = 0.005;
  float closest_depth = texture(shadow_map, coord.xy).r;
  float current_depth = coord.z;
  return current_depth - bias > closest_depth;
}

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

  if (in_shadow(shadow_coord)) {
    fragment_color = 0.5 * vec4(fragment_color.rgb, 1);
  }
}
)zzz"
