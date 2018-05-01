R"zzz(
#version 330 core

const int NUM_LIGHTS = 2;

uniform vec4 light_pos[NUM_LIGHTS];
uniform sampler2D pos_tex;
uniform sampler2D normal_tex;
uniform sampler2D diffuse_tex;
uniform sampler2D ao_tex;
uniform mat4 view;

in vec2 tex_coord;
out vec4 fragment_color;

void main() {
  vec3 pos = texture(pos_tex, tex_coord).rgb;
  vec3 normal = texture(normal_tex, tex_coord).rgb;
  vec3 diffuse = texture(diffuse_tex, tex_coord).rgb;
  float ao = texture(ao_tex, tex_coord).r;
  
  vec3 ambient = vec3(0.3 * diffuse);

  vec3 color = ambient;
  for (int i = 0; i < NUM_LIGHTS; i++) {
    vec3 light_direction = (view * light_pos[i]).xyz - pos;
    float dot_nl = dot(normalize(light_direction), normalize(normal));
    dot_nl = clamp(dot_nl, 0.0, 1.0);
    color += clamp(dot_nl * diffuse, 0.0, 1.0);
  }

  fragment_color = vec4(ao * clamp(color, 0, 1), 1);
}

)zzz"