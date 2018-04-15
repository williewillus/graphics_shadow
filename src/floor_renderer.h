#ifndef FLOOR_RENDERER_H
#define FLOOR_RENDERER_H
#pragma once
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
public:
  FloorRenderer();
  void draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& light_pos, const glm::mat4& depthMVP);
  void draw_shadow();
};
#endif
