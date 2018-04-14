#ifndef OBJ_RENDERER_H
#define OBJ_RENDERER_H
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class ObjRenderer {
  bool has_object = false;
  std::vector<glm::vec3> obj_vertices;
  std::vector<glm::uvec3> obj_faces;

  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  GLuint program = -1;

  GLuint projection_loc = -1;
  GLuint view_loc = -1;
  GLuint light_pos_loc = -1;

public:
  ObjRenderer();

  bool load(const std::string& file);
  void draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& light_pos);
};
#endif
