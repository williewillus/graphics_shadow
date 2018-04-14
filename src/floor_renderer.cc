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
  GLuint vs, fs, gs;

  CHECK_GL_ERROR(vs = glCreateShader(GL_VERTEX_SHADER));
  CHECK_GL_ERROR(glShaderSource(vs, 1, &floor_vert, nullptr));
  glCompileShader(vs);
  CHECK_GL_SHADER_ERROR(vs);

  CHECK_GL_ERROR(gs = glCreateShader(GL_GEOMETRY_SHADER));
  CHECK_GL_ERROR(glShaderSource(gs, 1, &floor_geom, nullptr));
  glCompileShader(gs);
  CHECK_GL_SHADER_ERROR(gs);

  CHECK_GL_ERROR(fs = glCreateShader(GL_FRAGMENT_SHADER));
  CHECK_GL_ERROR(glShaderSource(fs, 1, &floor_frag, nullptr));
  glCompileShader(fs);
  CHECK_GL_SHADER_ERROR(fs);

  CHECK_GL_ERROR(program = glCreateProgram());
  CHECK_GL_ERROR(glAttachShader(program, vs));
  CHECK_GL_ERROR(glAttachShader(program, gs));
  CHECK_GL_ERROR(glAttachShader(program, fs));

  CHECK_GL_ERROR(glBindAttribLocation(program, 0, "vertex_pos"));
  CHECK_GL_ERROR(glBindFragDataLocation(program, 0, "fragment_color"));

  glLinkProgram(program);
  CHECK_GL_PROGRAM_ERROR(program);

  CHECK_GL_ERROR(projection_loc = glGetUniformLocation(program, "projection"));
  CHECK_GL_ERROR(view_loc = glGetUniformLocation(program, "view"));
  CHECK_GL_ERROR(light_pos_loc = glGetUniformLocation(program, "light_pos"));

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

void FloorRenderer::draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& light_pos) {
  CHECK_GL_ERROR(glBindVertexArray(vao));
  CHECK_GL_ERROR(glUseProgram(program));

  // std::cout << "view loc " << view_loc << std::endl;
  // std::cout << "light pos loc " << light_pos_loc << std::endl;
  CHECK_GL_ERROR(glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &projection[0][0]));
  CHECK_GL_ERROR(glUniformMatrix4fv(view_loc, 1, GL_FALSE, &view[0][0]));
  CHECK_GL_ERROR(glUniform4fv(light_pos_loc, 1, &light_pos[0]));

  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0));
}
