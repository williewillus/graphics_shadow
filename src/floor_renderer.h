#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "config.h"

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
  GLuint program = -1;

public:
  FloorRenderer();
  void draw();
};
