R"zzz(
#version 330 core

in vec4 normal;
in vec3 world_position;

const int NUM_LIGHTS = 2;
uniform vec4 light_pos[NUM_LIGHTS];

out vec4 fragment_color;

void main() {
  vec3 base_color = vec3(0.0, 0.0, 1.0);
  vec3 color = vec3(0, 0, 0);

  for (int i = 0; i < NUM_LIGHTS; i++) {
    vec4 light_direction = light_pos[i] - vec4(world_position, 1);
    float dot_nl = dot(normalize(light_direction), normalize(normal));
    dot_nl = clamp(dot_nl, 0.0, 1.0);
    color += clamp(dot_nl * base_color, 0.0, 1.0);
  }
	
  // fragment_color = vec4(0, 0, 1, 1); 
  fragment_color = vec4(clamp(color, 0, 1), 1);
}
)zzz"
