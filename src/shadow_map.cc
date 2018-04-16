#include "shadow_map.h"
#include "config.h"
#include <debuggl.h>
#include <iostream>

ShadowMap::ShadowMap(GLuint depth_tex_, GLuint layer) {
  CHECK_GL_ERROR(glGenFramebuffers(1, &fbo));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
  CHECK_GL_ERROR(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex_, 0, layer));
  CHECK_GL_ERROR(glDrawBuffer(GL_NONE));
  CHECK_GL_ERROR(glReadBuffer(GL_NONE));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void ShadowMap::begin_capture() {
  CHECK_GL_ERROR(glViewport(0, 0, WIDTH, HEIGHT));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
  CHECK_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));
}
