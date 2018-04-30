R"zzz(
#version 330 core
layout (location = 0) out vec4 pos;
layout (location = 1) out vec4 normal;

in vec3 camera_normal;
in vec3 camera_position;

void main() {
  pos = vec4(camera_position, 1);
  normal = vec4(camera_normal, 1);
}

)zzz"
