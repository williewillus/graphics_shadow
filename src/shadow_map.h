#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class ShadowMap {
  GLuint fbo;
 public:
  static constexpr GLuint WIDTH = 2048;
  static constexpr GLuint HEIGHT = 2048;
  
  ShadowMap(GLuint depth_tex_, GLuint layer);

  void begin_capture();
};
