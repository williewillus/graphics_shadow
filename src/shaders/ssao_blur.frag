R"zzz(
#version 330 core
in vec2 tex_coord;
uniform sampler2D to_blur;
out float fragment_color;

const int RADIUS = 2;

void main() {
  vec2 pixel_to_uv = 1.0 / vec2(textureSize(to_blur, 0));
  float accum = 0.0;
  for (int dx = -RADIUS; dx < RADIUS; dx++) {
    for (int dy = -RADIUS; dy < RADIUS; dy++) {
      vec2 offset_uv = pixel_to_uv * vec2(float(dx), float(dy));
      accum += texture(to_blur, tex_coord + offset_uv).r;
    }
  }
  fragment_color = accum / (2*RADIUS * 2*RADIUS);
}

)zzz"
