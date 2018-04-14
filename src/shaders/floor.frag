R"zzz(
#version 420 core

in vec4 light_direction;
in vec3 world_position;
in vec4 shadowCoord;

out vec4 fragment_color;

uniform sampler2D shadowMap;

bool inShadow(vec4 coord_) {
  vec3 coord = coord_.xyz / coord_.w;
  coord = coord * 0.5 + 0.5;
  if (coord.x < 0 || coord.x > 1 || coord.y < 0 || coord.y > 1) {
    return false;
  }
  float closestDepth = texture(shadowMap, coord.xy).r;
  float currentDepth = coord.z;
  return currentDepth > closestDepth;
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

  if (inShadow(shadowCoord)) {
    fragment_color = 0.5 * vec4(fragment_color.rgb, 1);
  }
}
)zzz"
