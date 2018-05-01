#pragma once
#include "config.h"
#include "preview_renderer.h"
#include <glm/glm.hpp>
#include <random>
#include <vector>

class SSAOManager {
  static constexpr unsigned NUM_SAMPLES = 64;
  static constexpr unsigned NOISE_SIZE = 4;
  GLuint gbuffer, ssao_fbo, ssao_tex, ssao_blur_fbo, ssao_blur_tex;
  GLuint pos_tex, normal_tex, noise_tex, diffuse_tex;
  ShaderProgram ssao_program, ssao_blur_program, ssao_finalize_program, ssao_test_program;
  std::vector<glm::vec3> samples;
 public:
  SSAOManager(unsigned width, unsigned height);
  void begin_capture_geometry();
  void finish_render(const glm::mat4& projection, const glm::mat4& view, const std::array<glm::vec4, NUM_LIGHTS> light_pos, PreviewRenderer& pr);
};
