R"zzz(
#version 330 core
layout (location = 0) out vec3 pos;
layout (location = 1) out vec3 normal;

in vec3 camera_normal;
in vec3 camera_position;

void main() {
  pos = camera_position;
  normal = camera_normal;
}

)zzz"
