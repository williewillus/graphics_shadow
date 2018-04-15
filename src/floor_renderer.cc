#include "floor_renderer.h"
#include <iostream>
#include <debuggl.h>
#include <glm/ext.hpp>

FloorRenderer::FloorRenderer() {
  // init shaders
  const char* floor_vert = 
  #include "shaders/floor.vert"
  ;
  const char* floor_geom = 
  #include "shaders/floor.geom"
  ;
  const char* floor_frag = 
  #include "shaders/floor.frag"
  ;

  program
    .addVsh(floor_vert)
    .addGsh(floor_geom)
    .addFsh(floor_frag)
    .build({"projection", "view", "light_pos", "depthMVP" });

  // init VAO/VBO/EBO and upload data
  CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
  CHECK_GL_ERROR(glBindVertexArray(vao));

  CHECK_GL_ERROR(glGenBuffers(1, &vbo));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 3, &FLOOR_VERTICES[0], GL_STATIC_DRAW));

  CHECK_GL_ERROR(glGenBuffers(1, &ebo));
  CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
  CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 2 * 3, &FLOOR_FACES[0], GL_STATIC_DRAW));

  CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
  CHECK_GL_ERROR(glEnableVertexAttribArray(0));

  // cleanup
  CHECK_GL_ERROR(glBindVertexArray(0));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void FloorRenderer::draw_shadow() {
  CHECK_GL_ERROR(glBindVertexArray(vao));
  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0));
}

void FloorRenderer::draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& light_pos, const glm::mat4& depthMVP) {
  CHECK_GL_ERROR(glBindVertexArray(vao));

  program.activate();
  CHECK_GL_ERROR(glUniformMatrix4fv(program.getUniform("projection"), 1, GL_FALSE, &projection[0][0]));
  CHECK_GL_ERROR(glUniformMatrix4fv(program.getUniform("view"), 1, GL_FALSE, &view[0][0]));
  CHECK_GL_ERROR(glUniformMatrix4fv(program.getUniform("depthMVP"), 1, GL_FALSE, &depthMVP[0][0]));
  CHECK_GL_ERROR(glUniform4fv(program.getUniform("light_pos"), 1, &light_pos[0]));

  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0));
}
