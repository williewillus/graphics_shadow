R"zzz(
#version 330 core
layout (location = 0) out vec4 pos;
layout (location = 1) out vec4 normal;
layout (location = 2) out vec4 diffuse;

in vec3 camera_normal;
in vec3 camera_position;

void main() {
  pos = vec4(camera_position, 1);
  normal = vec4(camera_normal, 1);
  diffuse = vec4(0, 0, 1, 1);
}

)zzz"
