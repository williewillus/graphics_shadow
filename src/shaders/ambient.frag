R"zzz(
#version 330 core

out vec4 fragment_color;

const vec3 AMBIENT = vec3(0.1, 0.1, 0.1);
void main() {
  fragment_color = vec4(AMBIENT, 1);
}

)zzz"
