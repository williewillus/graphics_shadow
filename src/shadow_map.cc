#include "shadow_map.h"
#include "config.h"
#include <debuggl.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

ShadowMap::ShadowMap() {
  CHECK_GL_ERROR(glGenFramebuffers(1, &fbo));
  CHECK_GL_ERROR(glGenTextures(1, &depth_tex));
  CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, depth_tex));
  CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, width, height, 0 ,GL_DEPTH_COMPONENT, GL_FLOAT, 0));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
  CHECK_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0));
  CHECK_GL_ERROR(glDrawBuffer(GL_NONE));
  CHECK_GL_ERROR(glReadBuffer(GL_NONE));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

glm::mat4 ShadowMap::begin_capture(const glm::vec3& light_pos, const glm::vec3& light_dir) {
  CHECK_GL_ERROR(glViewport(0, 0, width, height));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
  CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, depth_tex));
  CHECK_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));

  glm::vec3 center = light_pos + 0.5f * light_dir;
  return glm::lookAt(light_pos, center, glm::vec3(0, 1, 0));
}
