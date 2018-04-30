R"zzz(
#version 330 core
const int NUM_SAMPLES = 64;
const int NOISE_SIZE = 4;
const float radius = 0.5;
const float bias = 0.025;
const vec2 noise_rescale = vec2(1280.0/float(NOISE_SIZE), 720.0/float(NOISE_SIZE));

in vec2 tex_coord;

uniform sampler2D pos_tex;
uniform sampler2D normal_tex;
uniform sampler2D noise_tex;
uniform vec3 offsets[NUM_SAMPLES];
uniform mat4 projection;

out vec4 fragment_color;

void main() {
  vec3 pos = texture(pos_tex, tex_coord).xyz;
  vec3 normal = texture(normal_tex, tex_coord).rgb;
  // tex_coord is 0-1 so would just go over the texture once. We instead want to tile this texture many times across the screen, so inflate the UV's so the texture wraps around.
  vec3 noise = texture(noise_tex, tex_coord * noise_rescale).xyz;
  
  // create matrix from tangent space (space of `samples`) to camera space (space of `pos`)
  vec3 tangent = normalize(noise - (normal * dot(noise, normal))); // apply noise
  vec3 up = cross(normal, tangent);
  mat3 tangent_to_camera = mat3(tangent, up, normal);

  float occlusion = 0.0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    vec3 sample_camera = pos + radius * (tangent_to_camera * offsets[i]);
    vec4 sample_ndc = projection * vec4(sample_camera, 1.0);
    sample_ndc /= sample_ndc.w;
    vec2 sample_uv = (sample_ndc.xy + 1.0) / 2.0;

    float neighbor_depth = texture(pos_tex, sample_uv).z;
    if (neighbor_depth >= sample_camera.z + bias) {
      // only contribute if our fragment's depth is "close enough" to neighbor's depth
      occlusion += smoothstep(0.0, 1.0, radius / abs(pos.z - neighbor_depth));
    }
  }
  occlusion /= NUM_SAMPLES;
  fragment_color = vec4(1 - occlusion, 0, 0, 1);
}
)zzz"
