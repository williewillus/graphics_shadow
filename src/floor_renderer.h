#ifndef FLOOR_RENDERER_H
#define FLOOR_RENDERER_H
#pragma once
#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "config.h"
#include "shader.h"

class FloorRenderer {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  ShaderProgram program;
  ShaderProgram ssao_program;
public:
  FloorRenderer();
  
  // light_idx = -1 if using shadow maps
  void draw(const glm::mat4& projection, const glm::mat4& view, const std::array<glm::vec4, NUM_LIGHTS>& light_pos, 
      const std::array<glm::mat4, NUM_LIGHTS>& depthMVP, const int light_idx);
  void draw_shadow();
  void draw_ssao(const glm::mat4& projection, const glm::mat4& view);
  void adjust_height(float objMin);
};
#endif
