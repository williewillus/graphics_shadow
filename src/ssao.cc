#include "ssao.h"
#include <iostream>
#include <debuggl.h>

SSAOManager::SSAOManager(unsigned width, unsigned height) {
  // Make FBO for deferred rendering
  {
    CHECK_GL_ERROR(glGenFramebuffers(1, &gbuffer));
    CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer));

    // Buffers for position and normal
    CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pos, 0));

    CHECK_GL_ERROR(glGenTextures(1, &normal));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, normal));
    CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal, 0));

    // Attach them
    GLuint attach[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    CHECK_GL_ERROR(glDrawBuffers(2, attach));
    
    // Depth buffer
    GLuint depth;
    CHECK_GL_ERROR(glGenRenderbuffers(1, &depth));
    CHECK_GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, depth));
    CHECK_GL_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
    CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth));
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("ssao geometry fbo not complete");
  }

  // FBO to hold SSAO data
  {
    CHECK_GL_ERROR(glGenFramebuffers(1, &ssao_fbo));
    CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo));
    CHECK_GL_ERROR(glGenTextures(1, &ssao_tex));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, ssao_tex));
    CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_tex, 0));
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("ssao geometry fbo not complete");
  }

  // FBO to hold blurred SSAO data (since blurring an FBO in-place is undefined behaviour)
  {
    CHECK_GL_ERROR(glGenFramebuffers(1, &ssao_blur_fbo));
    CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo));
    CHECK_GL_ERROR(glGenTextures(1, &ssao_blur_tex));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, ssao_blur_tex));
    CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_blur_tex, 0));
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("ssao geometry fbo not complete");
  }
  
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  std::uniform_real_distribution<float> dist(0.0, 1.0);
  std::default_random_engine gen;
  // Generate random offsets in a semicircle pointing in +Z direction to sample for SSAO
  {
    for (unsigned i = 0; i < 64; i++) {
      float x = dist(gen) * 2.0f - 1.0f;
      float y = dist(gen) * 2.0f - 1.0f;
      float z = dist(gen); // only a semicircle, so 0-1
      float magnitude = dist(gen);
      auto sample = magnitude * glm::normalize(glm::vec3 { x, y, z });
      float scale = static_cast<float>(i) / NUM_SAMPLES;
      scale = 0.1f + (scale*scale) * 0.9f; // quadratically scale, so more samples are near origin (the original pixel)
      samples.push_back(scale * sample);
    }
  }

  // Generate a few random rotation vectors, this will rotate the semicircle above randomly, which gives better results without increasing the sample size
  {
    std::vector<glm::vec3> noise;
    for (unsigned i = 0; i < NOISE_SIZE * NOISE_SIZE; i++) {
      float x = dist(gen) * 2.0f - 1.0f;
      float y = dist(gen) * 2.0f - 1.0f;
      noise.emplace_back(x, y, 0.0f);
    }
    CHECK_GL_ERROR(glGenTextures(1, &noise_tex));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, noise_tex));
    CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, NOISE_SIZE, NOISE_SIZE, 0, GL_RGB, GL_FLOAT, &noise[0]));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    // Note that since we specify WRAP for the texture, this will actually give jagged artifacts (since the rotations are in a fixed pattern)
    // That's why we blur the result afterward to get rid of it
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
  }
}

void SSAOManager::begin_capture_geometry() {
}

void SSAOManager::finish_render(PreviewRenderer& pr) {
}
