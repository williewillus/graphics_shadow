#include "preview_renderer.h"
#include <iostream>
#include <debuggl.h>
#include <glm/ext.hpp>

PreviewRenderer::PreviewRenderer() {
  // init shaders
  const char* preview_vert = 
  #include "shaders/preview.vert"
  ;
  const char* preview_frag = 
  #include "shaders/preview.frag"
  ;
  const char* volume_combine_frag =
  #include "shaders/volume_combine.frag"
  ;

  program
    .addVsh(preview_vert)
    .addFsh(preview_frag)
    .build({ "near_plane", "far_plane", "idx" });

  combine_program
    .addVsh(preview_vert)
    .addFsh(volume_combine_frag)
    .build({});

  // init VAO/VBO/EBO and upload data
  CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
  CHECK_GL_ERROR(glBindVertexArray(vao));

  CHECK_GL_ERROR(glGenBuffers(1, &vbo));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 3, &PREVIEW_VERTICES[0], GL_STATIC_DRAW));

  CHECK_GL_ERROR(glGenBuffers(1, &ebo));
  CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
  CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 2 * 3, &PREVIEW_FACES[0], GL_STATIC_DRAW));

  CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
  CHECK_GL_ERROR(glEnableVertexAttribArray(0));

  // cleanup
  CHECK_GL_ERROR(glBindVertexArray(0));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void PreviewRenderer::draw(const float& kNear, const float& kFar, unsigned current_preview) {
  program.activate();
  CHECK_GL_ERROR(glUniform1fv(program.getUniform("near_plane"), 1, &kNear));
  CHECK_GL_ERROR(glUniform1fv(program.getUniform("far_plane"), 1, &kFar));
  CHECK_GL_ERROR(glUniform1i(program.getUniform("idx"), current_preview));

  draw_quad();
}

void PreviewRenderer::draw_quad() {
  CHECK_GL_ERROR(glBindVertexArray(vao));
  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0));
}

void PreviewRenderer::draw_combine() {
  combine_program.activate();
  draw_quad();
}
