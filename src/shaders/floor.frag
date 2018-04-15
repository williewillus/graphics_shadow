R"zzz(
#version 330 core

in vec4 light_direction;
in vec3 world_position;
in vec4 shadow_coord;

out vec4 fragment_color;

uniform sampler2D shadow_map;

float compute_shadow(vec4 coord_) {
  vec3 coord = coord_.xyz / coord_.w;
  coord = coord * 0.5 + 0.5;
  if (coord.x < 0 || coord.x > 1 || coord.y < 0 || coord.y > 1) {
    return 0.0;    
  }

  float shadow = 0.0;
  float bias = 0.005;

  vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
  float current_depth = coord.z;

  for(int x = -2; x <= 2; x++) {
      for(int y = -2; y <= 2; y++) {
          float neighbor_depth = texture(shadow_map, coord.xy + vec2(x, y) * texel_size).r; 
          shadow += current_depth - bias > neighbor_depth ? 1.0 : 0.0;        
      }    
  }
  shadow /= 25.0;
  return shadow;
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

  float shadow = compute_shadow(shadow_coord);
  fragment_color = vec4((1 - shadow) * fragment_color.rgb, 1.0);
}
)zzz"
