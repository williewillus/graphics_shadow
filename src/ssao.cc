#include "ssao.h"
#include <iostream>
#include <debuggl.h>
#include <jpegio.h>

SSAOManager::SSAOManager(unsigned width, unsigned height) {
  // Make FBO for deferred rendering
  {
    CHECK_GL_ERROR(glGenFramebuffers(1, &gbuffer));
    CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer));

    // Buffers for position and normal
    CHECK_GL_ERROR(glGenTextures(1, &pos_tex));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, pos_tex));
    CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pos_tex, 0));

    CHECK_GL_ERROR(glGenTextures(1, &normal_tex));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, normal_tex));
    CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_tex, 0));

    CHECK_GL_ERROR(glGenTextures(1, &diffuse_tex));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, diffuse_tex));
    CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, diffuse_tex, 0));
    
    // Attach them
    GLuint attach[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    CHECK_GL_ERROR(glDrawBuffers(3, attach));
    
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
      throw std::runtime_error("ssao main fbo not complete");
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
      throw std::runtime_error("ssao blur fbo not complete");
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
    CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, NOISE_SIZE, NOISE_SIZE, 0, GL_RGB, GL_FLOAT, &noise[0]));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    // Note that since we specify WRAP for the texture, this will actually give jagged artifacts (since the rotations are in a fixed pattern)
    // That's why we blur the result afterward to get rid of it
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
  }

  const char* preview_vert =
  #include "shaders/preview.vert"
  ;
  const char* ssao_frag =
  #include "shaders/ssao.frag"
  ;
  const char* ssao_blur_frag =
  #include "shaders/ssao_blur.frag"
  ;
  const char* ssao_test_frag =
  #include "shaders/ssao_test.frag"
  ;
  const char* ssao_finalize_frag =
  #include "shaders/ssao_finalize.frag"
  ;
  
  ssao_program
    .addVsh(preview_vert)
    .addFsh(ssao_frag)
    .build({ "projection", "pos_tex", "normal_tex", "noise_tex" });

  ssao_blur_program
    .addVsh(preview_vert)
    .addFsh(ssao_blur_frag)
    .build({ "to_blur" });

  ssao_finalize_program
    .addVsh(preview_vert)
    .addFsh(ssao_finalize_frag)
    .build({ "view", "pos_tex", "normal_tex", "diffuse_tex", "ao_tex" });

  ssao_test_program
    .addVsh(preview_vert)
    .addFsh(ssao_test_frag)
    .build({});
}

void SSAOManager::begin_capture_geometry() {
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, gbuffer));
  CHECK_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void SSAOManager::finish_render(const glm::mat4& projection, const glm::mat4& view, const std::array<glm::vec4, NUM_LIGHTS> light_pos, PreviewRenderer& pr, bool debug) {
  // SSAO pass
  {
    CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo));
    CHECK_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));
    ssao_program.activate();
    CHECK_GL_ERROR(glUniform1i(ssao_program.getUniform("pos_tex"), 0));
    CHECK_GL_ERROR(glUniform1i(ssao_program.getUniform("normal_tex"), 1));
    CHECK_GL_ERROR(glUniform1i(ssao_program.getUniform("noise_tex"), 2));
    CHECK_GL_ERROR(glUniformMatrix4fv(ssao_program.getUniform("projection"), 1, GL_FALSE, &projection[0][0]));
    for (unsigned i = 0; i < NUM_SAMPLES; i++) {
      std::string name = "offsets[";
      name += std::to_string(i);
      name += "]";
      CHECK_GL_ERROR(glUniform3fv(ssao_program.get_uniform_direct(name), 1, &samples[i][0]));
    }
    CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, pos_tex));
    CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE1));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, normal_tex));
    CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE2));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, noise_tex));
    pr.draw_quad();
  }

  // Blur pass
  {
    CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo));
    CHECK_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));
    ssao_blur_program.activate();
    CHECK_GL_ERROR(glUniform1i(ssao_blur_program.getUniform("to_blur"), 0));
    CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0));
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, ssao_tex));
    pr.draw_quad(); 
  }

  // Final pass
  {
    CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    CHECK_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    if (debug) {
      ssao_test_program.activate();
      CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, ssao_blur_tex));
    } else {
      ssao_finalize_program.activate();
      for (unsigned i = 0; i < light_pos.size(); i++) {
	std::string loc_name = "light_pos[" +  std::to_string(i) + "]";
	auto loc = ssao_finalize_program.get_uniform_direct(loc_name);
	CHECK_GL_ERROR(glUniform4fv(loc, 1, &light_pos[i][0]));
      }
      CHECK_GL_ERROR(glUniform1i(ssao_finalize_program.getUniform("pos_tex"), 0));
      CHECK_GL_ERROR(glUniform1i(ssao_finalize_program.getUniform("normal_tex"), 1));
      CHECK_GL_ERROR(glUniform1i(ssao_finalize_program.getUniform("diffuse_tex"), 2));
      CHECK_GL_ERROR(glUniform1i(ssao_finalize_program.getUniform("ao_tex"), 3));
      CHECK_GL_ERROR(glUniformMatrix4fv(ssao_finalize_program.getUniform("view"), 1, GL_FALSE, &view[0][0]));

      CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0));
      CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, pos_tex));
      CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE1));
      CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, normal_tex));
      CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE2));
      CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, diffuse_tex));
      CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE3));
      CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, ssao_blur_tex));
    }

    pr.draw_quad();
    CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0));
  }
}
