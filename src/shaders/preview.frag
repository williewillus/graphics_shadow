R"zzz(#version 330 core
out vec4 fragment_color;
  
in vec2 tex_coord;

uniform sampler2D depthMap;

void main()
{             
    float d_x = min(tex_coord.x, 1.0 - tex_coord.x);
    float d_y = min(tex_coord.y, 1.0 - tex_coord.y);
    if (d_x < 0.02 || d_y < 0.02) {
    	fragment_color = vec4(0.0, 0.0, 1.0, 0.3);
    } else {
      float depthValue = texture(depthMap, tex_coord).r;
      fragment_color = vec4(vec3(depthValue), 1.0);
    }
}  
)zzz"
