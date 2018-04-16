R"zzz(#version 330 core
out vec4 fragment_color;
  
in vec2 tex_coord;

uniform sampler2DArray depth_map;
uniform float near_plane;
uniform float far_plane;
uniform int idx;

// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float linearize_depth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{             
    float d_x = min(tex_coord.x, 1.0 - tex_coord.x);
    float d_y = min(tex_coord.y, 1.0 - tex_coord.y);
    if (d_x < 0.02 || d_y < 0.02) {
    	fragment_color = vec4(0.0, 0.0, 1.0, 0.3);
    } else {
      float depth_value = texture(depth_map, vec3(tex_coord, float(idx))).r;
      fragment_color = vec4(vec3(linearize_depth(depth_value) / far_plane), 1.0); // perspective
    }
}  
)zzz"
