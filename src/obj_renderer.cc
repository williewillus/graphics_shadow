#include "obj_renderer.h"
#include <iostream>
#include <fstream>
#include <debuggl.h>
#include <glm/ext.hpp>

ObjRenderer::ObjRenderer() {
  const char* obj_vert = 
  #include "shaders/obj.vert"
  ;
  const char* obj_geom = 
  #include "shaders/obj.geom"
  ;
  const char* obj_frag = 
  #include "shaders/obj.frag"
  ;

  program
    .addVsh(obj_vert)
    .addGsh(obj_geom)
    .addFsh(obj_frag)
    .build({ "projection", "view", "light_pos" });
}

bool ObjRenderer::load(const std::string& file) {
  std::ifstream fin (file);
  char type;
  int x, y, z;
  float v1, v2, v3;

  if (!fin.good()) 
    return false;

  while(fin >> type) {
    if (type == 'v') {
      fin >> v1 >> v2 >> v3;
      obj_vertices.push_back({ v1, v2, v3 });
    } else if (type == 'f') {
      fin >> x >> y >> z;
      obj_faces.push_back({ x - 1, y - 1, z - 1 });
    } else {
      obj_vertices.clear();
      obj_faces.clear();
      return false;
    }
  }

  // init VAO/VBO/EBO and upload data
  CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
  CHECK_GL_ERROR(glBindVertexArray(vao));

  CHECK_GL_ERROR(glGenBuffers(1, &vbo));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj_vertices.size() * 3, obj_vertices.data(), GL_STATIC_DRAW));

  CHECK_GL_ERROR(glGenBuffers(1, &ebo));
  CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
  CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * obj_faces.size() * 3, obj_faces.data(), GL_STATIC_DRAW));

  CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
  CHECK_GL_ERROR(glEnableVertexAttribArray(0));

  // cleanup
  CHECK_GL_ERROR(glBindVertexArray(0));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

  std::cout << "Successfully loaded " << obj_vertices.size() << " vertices and " << obj_faces.size() << " faces from obj file: " << file << std::endl;
  has_object = true;  
  return true;
}

void ObjRenderer::draw_shadow() {
  if (!has_object) {
    return;
  }

  CHECK_GL_ERROR(glBindVertexArray(vao));
  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, obj_faces.size() * 3, GL_UNSIGNED_INT, 0));
}

void ObjRenderer::draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& light_pos) {
  if (!has_object) {
    return;
  }
  CHECK_GL_ERROR(glBindVertexArray(vao));
  program.activate();

  CHECK_GL_ERROR(glUniformMatrix4fv(program.getUniform("projection"), 1, GL_FALSE, &projection[0][0]));
  CHECK_GL_ERROR(glUniformMatrix4fv(program.getUniform("view"), 1, GL_FALSE, &view[0][0]));
  CHECK_GL_ERROR(glUniform4fv(program.getUniform("light_pos"), 1, &light_pos[0]));

  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, obj_faces.size() * 3, GL_UNSIGNED_INT, 0));
}
