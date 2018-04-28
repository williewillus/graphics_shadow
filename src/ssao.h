#pragma once
#include "preview_renderer.h"
#include <glm/glm.hpp>
#include <random>
#include <vector>

class SSAOManager {
  static constexpr unsigned NUM_SAMPLES = 64;
  static constexpr unsigned NOISE_SIZE = 4;
  GLuint gbuffer, pos, normal;
  GLuint ssao_fbo, ssao_tex, ssao_blur_fbo, ssao_blur_tex;
  GLuint noise_tex;
  ShaderProgram ssao_geom_program, ssao_program, ssao_blur_program;
  std::vector<glm::vec3> samples;
 public:
  SSAOManager(unsigned width, unsigned height);
  void begin_capture_geometry(const glm::mat4& projection, const glm::mat4& view);
  void finish_render(PreviewRenderer& pr);
};
