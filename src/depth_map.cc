#include "depth_map.h"
#include "config.h"
#include <debuggl.h>
#include <iostream>

DepthMap::DepthMap(GLuint width, GLuint height, GLuint depth_tex_, GLuint layer) : width(width), height(height) {
  CHECK_GL_ERROR(glGenFramebuffers(1, &fbo));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
  CHECK_GL_ERROR(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex_, 0, layer));
  CHECK_GL_ERROR(glDrawBuffer(GL_NONE));
  CHECK_GL_ERROR(glReadBuffer(GL_NONE));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void DepthMap::begin_capture() {
  CHECK_GL_ERROR(glViewport(0, 0, width, height));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
  CHECK_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));
}

TextureToRender::TextureToRender(GLuint width, GLuint height, GLuint tex_, GLuint layer) : width(width), height(height) {
  CHECK_GL_ERROR(glGenFramebuffers(1, &fbo));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

  // allocate a depth buffer (idk if actually needed)
  GLuint dep_;
  CHECK_GL_ERROR(glGenRenderbuffers(1, &dep_));
  CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, dep_));
  CHECK_GL_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                                       width, height));
  CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                           GL_RENDERBUFFER, dep_));

  // attach to given texture layer
  CHECK_GL_ERROR(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_, 0, layer));
  //CHECK_GL_ERROR(glReadBuffer(GL_NONE));
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
  
void TextureToRender::begin_capture() {
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
  CHECK_GL_ERROR(glViewport(0, 0, width, height));

  CHECK_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
  CHECK_GL_ERROR(glEnable(GL_DEPTH_TEST));
  CHECK_GL_ERROR(glEnable(GL_MULTISAMPLE));
  CHECK_GL_ERROR(glEnable(GL_BLEND));
  CHECK_GL_ERROR(glEnable(GL_CULL_FACE));
  CHECK_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
  CHECK_GL_ERROR(glDepthFunc(GL_LEQUAL));
  CHECK_GL_ERROR(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  CHECK_GL_ERROR(glCullFace(GL_BACK));


  CHECK_GL_ERROR(glDepthMask(GL_TRUE));
  CHECK_GL_ERROR(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
  CHECK_GL_ERROR(glStencilMask(GL_TRUE));
  CHECK_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}
