R"zzz(#version 330 core

uniform mat4 depthMVP;

in vec3 vertex_position;

void main() {
	gl_Position = depthMVP * vec4(vertex_position, 1.0);
}
)zzz"
