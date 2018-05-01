#ifndef FLOOR_RENDERER_H
#define FLOOR_RENDERER_H
#pragma once
#include <array>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "config.h"
#include "shader.h"

static const glm::vec3 FLOOR_VERTICES[] = {
  { kFloorXMin, kFloorY, kFloorZMax },
  { kFloorXMax, kFloorY, kFloorZMax },
  { kFloorXMax, kFloorY, kFloorZMin },
  { kFloorXMin, kFloorY, kFloorZMin }
};

static const glm::uvec3 FLOOR_FACES[] = {
  { 0, 1, 2 },
  { 2, 3, 0 }
};


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
};
#endif
