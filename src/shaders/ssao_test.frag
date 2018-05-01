R"zzz(#version 330 core

in vec2 tex_coord;
uniform sampler2D ao_tex;
out vec4 fragment_color;

void main() {
  vec4 color = texture(ao_tex, tex_coord);
  fragment_color = vec4(color.rrr, 1);
}

)zzz"
