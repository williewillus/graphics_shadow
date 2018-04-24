#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "shader.h"

static const glm::vec3 PREVIEW_VERTICES[] = {
  { -1, -1, 0 },
  { -1,  1, 0 },
  {  1, -1, 0 },
  {  1,  1, 0 }
};

static const glm::uvec3 PREVIEW_FACES[] = {
  { 0, 3, 1 },
  { 0, 2, 3 }
};

class PreviewRenderer {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  ShaderProgram program;
  ShaderProgram combine_program;
public:
  PreviewRenderer();
  void draw(const float& kNear, const float& kFar, unsigned current_preview);
  void draw_combine();
};
