R"zzz(
#version 330 core

in vec2 tex_coord;
out vec4 fragment_color;
uniform sampler2DArray tex;
const int NUM_LIGHTS = 2;

void main() {
  fragment_color = vec4(0, 0, 0, 1);
  for (int i = 0; i < NUM_LIGHTS; i++) {
    fragment_color += texture(tex, vec3(tex_coord, float(i)));
  }
  fragment_color = vec4(clamp(fragment_color.xyz, 0, 1), 1);
}

)zzz"
