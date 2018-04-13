#include "floor_renderer.h"
#include "shader_sources.h"
#include <iostream>
#include <debuggl.h>
#include <glm/ext.hpp>

FloorRenderer::FloorRenderer() {
  // init shaders
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

  // init VAO/VBO and upload data
  CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
  CHECK_GL_ERROR(glBindVertexArray(vao));

  CHECK_GL_ERROR(glGenBuffers(1, &vbo));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 3, &FLOOR_VERTICES[0], GL_STATIC_DRAW));

  CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
  CHECK_GL_ERROR(glEnableVertexAttribArray(0));
  CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(uint32_t) * 2 * 3, &FLOOR_FACES[0], GL_STATIC_DRAW));

  // cleanup
  CHECK_GL_ERROR(glBindVertexArray(0));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void FloorRenderer::draw() {
  CHECK_GL_ERROR(glBindVertexArray(vao));
  CHECK_GL_ERROR(glUseProgram(program));
  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0));
}
