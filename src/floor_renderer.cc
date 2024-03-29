#include "floor_renderer.h"
#include <iostream>
#include <debuggl.h>
#include <glm/ext.hpp>

static const glm::uvec3 FLOOR_FACES[] = {
  { 0, 1, 2 },
  { 2, 3, 0 }
};

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
  const char* floor_ssao_frag =
  #include "shaders/floor_ssao.frag"
  ;

  program
    .addVsh(floor_vert)
    .addGsh(floor_geom)
    .addFsh(floor_frag)
    .build({"projection", "view", "light_pos", "depthMVP", "light_idx" });
  ssao_program
    .addVsh(floor_vert)
    .addGsh(floor_geom)
    .addFsh(floor_ssao_frag)
    .build({"projection", "view" }, { "pos", "normal", "diffuse" });

  // init VAO/VBO/EBO and upload data
  CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
  CHECK_GL_ERROR(glBindVertexArray(vao));

  CHECK_GL_ERROR(glGenBuffers(1, &vbo));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));

  CHECK_GL_ERROR(glGenBuffers(1, &ebo));
  CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
  CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 2 * 3, &FLOOR_FACES[0], GL_STATIC_DRAW));

  CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
  CHECK_GL_ERROR(glEnableVertexAttribArray(0));

  adjust_height(0.1f);

  // cleanup
  CHECK_GL_ERROR(glBindVertexArray(0));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void FloorRenderer::adjust_height(float objMin) {
  const std::array<glm::vec3, 4> floor_vertices = {
    glm::vec3 { kFloorXMin, objMin - 0.1f, kFloorZMax },
    glm::vec3 { kFloorXMax, objMin - 0.1f, kFloorZMax },
    glm::vec3 { kFloorXMax, objMin - 0.1f, kFloorZMin },
    glm::vec3 { kFloorXMin, objMin - 0.1f, kFloorZMin }
  };

  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 3, floor_vertices.data(), GL_STATIC_DRAW));
}

void FloorRenderer::draw_shadow() {
  CHECK_GL_ERROR(glBindVertexArray(vao));
  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0));
}

void FloorRenderer::draw_ssao(const glm::mat4& projection, const glm::mat4& view) {
  ssao_program.activate();
  CHECK_GL_ERROR(glUniformMatrix4fv(ssao_program.getUniform("projection"), 1, GL_FALSE, &projection[0][0]));
  CHECK_GL_ERROR(glUniformMatrix4fv(ssao_program.getUniform("view"), 1, GL_FALSE, &view[0][0]));
  draw_shadow();
}

void FloorRenderer::draw(const glm::mat4& projection, const glm::mat4& view, const std::array<glm::vec4, NUM_LIGHTS>& light_pos, 
    const std::array<glm::mat4, NUM_LIGHTS>& depthMVP, const int light_idx) {
  CHECK_GL_ERROR(glBindVertexArray(vao));

  program.activate();
  CHECK_GL_ERROR(glUniformMatrix4fv(program.getUniform("projection"), 1, GL_FALSE, &projection[0][0]));
  CHECK_GL_ERROR(glUniformMatrix4fv(program.getUniform("view"), 1, GL_FALSE, &view[0][0]));
  CHECK_GL_ERROR(glUniform1i(program.getUniform("light_idx"), light_idx));

  for (unsigned i = 0; i < light_pos.size(); i++) {
    std::string loc_name = "light_pos[" +  std::to_string(i) + "]";
    auto loc = program.get_uniform_direct(loc_name);
    CHECK_GL_ERROR(glUniform4fv(loc, 1, &light_pos[i][0]));
  }

  for (unsigned i = 0; i < depthMVP.size(); i++) {
    std::string loc_name = "depthMVP[";
    loc_name += std::to_string(i);
    loc_name += "]";

    auto loc = program.get_uniform_direct(loc_name);
    CHECK_GL_ERROR(glUniformMatrix4fv(loc, 1, GL_FALSE, &depthMVP[i][0][0]));
  }

  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0));
}
