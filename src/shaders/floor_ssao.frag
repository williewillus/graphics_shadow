R"zzz(
#version 330 core
layout (location = 0) out vec4 pos;
layout (location = 1) out vec4 normal;
layout (location = 2) out vec4 diffuse;

in vec3 world_position;
in vec3 camera_normal;
in vec3 camera_position;

const vec4 gray = vec4(0.97, 0.97, 0.97, 1.0);
const vec4 white = vec4(1.0, 1.0, 1.0, 1.0);

void main() {
  pos = vec4(camera_position, 1);
  normal = vec4(camera_normal, 1);

  if (floor(mod(world_position[0], 2)) == 1) {
    diffuse = floor(mod(world_position[2], 2)) == 0 ? white : gray;
  } else {
    diffuse = floor(mod(world_position[2], 2)) == 0 ? gray : white;
  }
}

)zzz"
