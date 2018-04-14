#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class ShadowMap {
  static constexpr int SHADOWMAP_TEXTURE_UNIT = 14;
  GLuint fbo;
  GLuint depth_tex;
  const GLuint width = 2048;
  const GLuint height = 2048;
 public:
  ShadowMap();

  void begin_capture();

  GLuint get_depth_texture() const { return depth_tex; };
};
