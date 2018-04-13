R"zzz(#version 330 core

uniform vec3 joint_trans[256];
uniform vec4 joint_rot[256];
uniform mat4 depthMVP;

in vec4 vertex_position;
in int jid0;
in int jid1;
in float w0;
in vec3 vector_from_joint0;
in vec3 vector_from_joint1;

vec3 qtransform(vec4 q, vec3 v) {
  return v + 2.0 * cross(cross(v, q.xyz) - q.w*v, q.xyz);
}

void main() {
  vec3 vert0 = qtransform(joint_rot[jid0], vector_from_joint0) + joint_trans[jid0];
  vec3 vert1 = (jid1 < 0) ? vec3(0, 0, 0) : qtransform(joint_rot[jid1], vector_from_joint1) + joint_trans[jid1];


  vec4 position = vec4(w0 * vert0 + (1 - w0) * vert1, 1);
	gl_Position = depthMVP * position;
	// gl_Position = depthMVP * vertex_position;
}
)zzz"
