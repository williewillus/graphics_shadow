R"zzz(#version 330 core
in vec4 vertex_position;
uniform mat4 orthomat;
uniform float frame_shift;
out vec2 tex_coord;
void main()
{
	tex_coord = (vertex_position.xy + 1) / 2.0f;
	vec4 pos = vertex_position;
	pos.y += frame_shift;
	gl_Position = orthomat * pos;
}
)zzz"
