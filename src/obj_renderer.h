#ifndef OBJ_RENDERER_H
#define OBJ_RENDERER_H
#pragma once

#include <algorithm>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <array>
#include "config.h"
#include "shader.h"

struct Edge {
  unsigned a, b;
  Edge(unsigned a_, unsigned b_) {
    a = std::min(a_, b_);
    b = std::max(a_, b_);
  }
  bool operator==(const Edge& o) const {
    return a == o.a && b == o.b;
  }
};

namespace std {
  template<> struct hash<Edge> {
    typedef Edge argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type v) const noexcept {
      std::size_t seed = 0;
      std::hash<unsigned> hasher;
      seed ^= hasher(v.a) + 0x9e3779b9 + (seed<<6) + (seed>>2);
      seed ^= hasher(v.b) + 0x9e3779b9 + (seed<<6) + (seed>>2);
      return seed;
    }
  };
}

class ObjRenderer {
  bool has_object = false;
  std::vector<glm::vec3> obj_vertices;
  std::vector<glm::uvec3> obj_faces;

  GLuint vao;
  GLuint silhouette_vao;

  ShaderProgram program;
  ShaderProgram volume_program;
  ShaderProgram ssao_program;
  float min_y = 0.1;
public:
  ObjRenderer();

  bool load(const std::string& file);

  // light_idx = -1 if using shadow maps
  void draw(const glm::mat4& projection, const glm::mat4& view, const std::array<glm::vec4, NUM_LIGHTS>& light_pos, 
      const std::array<glm::mat4, NUM_LIGHTS>& depthMVP, const int light_idx);
  void draw_volume(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& light_pos);
  void draw_shadow();
  void draw_ssao(const glm::mat4& projection, const glm::mat4& view);
  float get_min_y() const { return min_y; }
};
#endif
