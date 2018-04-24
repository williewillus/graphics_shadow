#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class DepthMap {
  GLuint fbo;
  GLuint width;
  GLuint height;
 public:
  DepthMap(GLuint width, GLuint height, GLuint depth_tex_, GLuint layer);

  void begin_capture();
};

class TextureToRender {
  GLuint fbo;
 public:
  TextureToRender(GLuint width, GLuint height, GLuint depth_tex_, GLuint layer);
  void begin_capture();
};
