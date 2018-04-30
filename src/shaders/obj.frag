R"zzz(
#version 330 core

in vec4 normal;
in vec3 world_position;

const int NUM_LIGHTS = 2;
uniform vec4 light_pos[NUM_LIGHTS];
uniform mat4 depthMVP[NUM_LIGHTS];
uniform sampler2DArray shadow_map;
uniform int use_shadow_map;
uniform int ambient;

out vec4 fragment_color;

float compute_shadow(vec4 coord_, int idx) {
  vec3 coord = coord_.xyz / coord_.w;
  coord = coord * 0.5 + 0.5;
  if (coord.x < 0 || coord.x > 1 || coord.y < 0 || coord.y > 1) {
    return 0.0;    
  }

  float shadow = 0.0;
  float bias = 0.00003;

  vec2 texel_size = 1.0 / textureSize(shadow_map, 0).xy;
  float current_depth = coord.z;

  for(int x = 0; x <= 0; x++) {
      for(int y = 0; y <= 0; y++) {
          vec3 uv = vec3(coord.xy + vec2(x, y) * texel_size, float(idx));
          float neighbor_depth = texture(shadow_map, uv).r; 
          shadow += current_depth - bias > neighbor_depth ? 1.0 : 0.0;        
      }    
  }
  return shadow;
}

void main() {
  vec3 base_color = vec3(0.0, 0.0, 1.0);
  vec3 color = vec3(0, 0, 0);

  if (ambient != 0) {
    fragment_color = vec4(0.15 * base_color, 1.0);
    return;
  }

  if (use_shadow_map == 1) {
    for (int i = 0; i < NUM_LIGHTS; i++) {
      vec4 light_direction = light_pos[i] - vec4(world_position, 1);
      float dot_nl = dot(normalize(light_direction), normalize(normal));
      dot_nl = clamp(dot_nl, 0.0, 1.0);

      vec4 shadow_coord = depthMVP[i] * vec4(world_position, 1);
      if (compute_shadow(shadow_coord, i) != 1.0) {
        color += clamp(dot_nl * base_color, 0.0, 1.0);
      }
    }
    color = clamp(color, 0.0, 1.0);
    fragment_color = vec4(color, 1.0);
  }
  else {
    for (int i = 0; i < NUM_LIGHTS; i++) {
      vec4 light_direction = light_pos[i] - vec4(world_position, 1);
      float dot_nl = dot(normalize(light_direction), normalize(normal));
      dot_nl = clamp(dot_nl, 0.0, 1.0);
      color += clamp(dot_nl * base_color, 0.0, 1.0);
    }
    color = clamp(color, 0.0, 1.0);
    fragment_color = vec4(color, 1.0);
  }
}
)zzz"
