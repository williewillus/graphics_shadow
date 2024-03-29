R"zzz(
#version 330 core
layout (location = 0) out vec4 pos;
layout (location = 1) out vec4 normal;
layout (location = 2) out vec4 diffuse;

in vec3 world_position;
in vec3 camera_normal;
in vec3 camera_position;


const vec4 color_a = vec4(0.48, 0.48, 0.48, 1.0);
const vec4 color_b = vec4(0.5, 0.5, 0.5, 1.0);

void main() {
  pos = vec4(camera_position, 1);
  normal = vec4(camera_normal, 1);

  if (floor(mod(world_position[0], 2)) == 1) {
    diffuse = floor(mod(world_position[2], 2)) == 0 ? color_a : color_b;
  } else {
    diffuse = floor(mod(world_position[2], 2)) == 0 ? color_b : color_a;
  }
}

)zzz"
