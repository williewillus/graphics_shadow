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

  // Returns: view matrix for this light
  glm::mat4 begin_capture(const glm::vec3& light_pos, const glm::vec3& light_dir);

  GLuint get_depth_texture() const { return depth_tex; };
};
