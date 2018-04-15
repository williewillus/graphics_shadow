R"zzz(#version 330 core
in vec4 vertex_position;
out vec2 tex_coord;
void main()
{
	tex_coord = (vertex_position.xy + 1) / 2.0f;
	gl_Position = vertex_position;
}
)zzz"
