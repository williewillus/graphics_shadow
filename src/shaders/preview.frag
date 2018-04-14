R"zzz(#version 330 core
out vec4 fragment_color;
  
in vec2 tex_coord;

uniform sampler2D depthMap;

void main()
{             
    float depthValue = texture(depthMap, tex_coord).r;
    fragment_color = vec4(vec3(depthValue), 1.0);
}  
)zzz"
