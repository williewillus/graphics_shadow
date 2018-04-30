#include "obj_renderer.h"
#include <iostream>
#include <fstream>
#include <debuggl.h>
#include <glm/ext.hpp>
#include <unordered_map>
#include <utility>

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
  const char* silhouette_frag =
  #include "shaders/silhouette.frag"
  ;
  const char* shadow_volume_geom =
  #include "shaders/shadow_volume.geom"
  ;

  program
    .addVsh(obj_vert)
    .addGsh(obj_geom)
    .addFsh(obj_frag)
    .build({"projection", "view", "light_pos", "depthMVP", "use_shadow_map", "ambient" });
  volume_program
    .addVsh(obj_vert)
    .addGsh(shadow_volume_geom)
    .addFsh(silhouette_frag)
    .build({ "projection", "view", "light_pos" });
}

// Return vertex index opposite to given face
static unsigned opposite(const glm::uvec3& face, const Edge& e) {
  for (unsigned i = 0; i < 3; i++) {
    if (face[i] != e.a && face[i] != e.b) {
      return face[i];
    }
  }

  throw std::runtime_error("impossible");
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

  // build edge -> two face idxs sharing it
  std::unordered_map<Edge, std::pair<unsigned, unsigned>> edge_to_face;
  for (unsigned i = 0; i < obj_faces.size(); i++) {
    const auto& face = obj_faces[i];
    for (unsigned j = 0; j < 3; j++) {
      Edge e { face[j], face[(j+1) % 3] };
      auto it = edge_to_face.find(e);

      if (it == edge_to_face.cend()) {
	edge_to_face[e] = std::make_pair(i, -1);
      } else if (it->second.second != -1) {
	throw std::runtime_error("more than two faces sharing an edge??");
      } else {
	it->second.second = i;
      }
    }
  }

  // If any edge has only 1 face, the mesh is not closed properly (TODO handle this smartly?)
  for (const auto& entry : edge_to_face) {
    if (entry.second.second == -1) {
      throw std::runtime_error("non-closed mesh, can't build adjacencies");
    }
  }

  // calculate final ebo values
  std::vector<unsigned> adjacency_idxs;
  adjacency_idxs.reserve(obj_faces.size() * 3 * 2); // each vertex also comes with adjacent vertex, so double
  for (unsigned i = 0; i < obj_faces.size(); i++) {
    const auto& face = obj_faces[i];
    for (unsigned j = 0; j < 3; j++) {
      Edge e { face[j], face[(j+1) % 3] };

      // Actual triangle vertex idx
      adjacency_idxs.push_back(face[j]);

      // Adjacent triangle vertex idx
      auto pair = edge_to_face.at(e);
      auto adjacent_face_idx = pair.first == i ? pair.second : pair.first;
      auto adjacent_face = obj_faces.at(adjacent_face_idx);
      auto opposite_vertex_idx = opposite(adjacent_face, e);
      adjacency_idxs.push_back(opposite_vertex_idx);
    }
  }

  // init VAO/VBO/EBO and upload data
  CHECK_GL_ERROR(glGenVertexArrays(1, &vao));
  CHECK_GL_ERROR(glBindVertexArray(vao));

  GLuint vbo, ebo;
  CHECK_GL_ERROR(glGenBuffers(1, &vbo));
  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj_vertices.size() * 3, obj_vertices.data(), GL_STATIC_DRAW));

  CHECK_GL_ERROR(glGenBuffers(1, &ebo));
  CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
  CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * obj_faces.size() * 3, obj_faces.data(), GL_STATIC_DRAW));

  CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
  CHECK_GL_ERROR(glEnableVertexAttribArray(0));

  // Make another for silhouette
  CHECK_GL_ERROR(glGenVertexArrays(1, &silhouette_vao));
  CHECK_GL_ERROR(glBindVertexArray(silhouette_vao));

  CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
  CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
  CHECK_GL_ERROR(glEnableVertexAttribArray(0));

  GLuint silhouette_ebo;
  CHECK_GL_ERROR(glGenBuffers(1, &silhouette_ebo));
  CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, silhouette_ebo));
  CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * adjacency_idxs.size(), adjacency_idxs.data(), GL_STATIC_DRAW));

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

void ObjRenderer::draw_volume(const glm::mat4& projection, const glm::mat4& view, const glm::vec4& light_pos) {
  if (!has_object) {
    return;
  }
  CHECK_GL_ERROR(glBindVertexArray(silhouette_vao));
  volume_program.activate();

  CHECK_GL_ERROR(glUniformMatrix4fv(volume_program.getUniform("projection"), 1, GL_FALSE, &projection[0][0]));
  CHECK_GL_ERROR(glUniformMatrix4fv(volume_program.getUniform("view"), 1, GL_FALSE, &view[0][0]));
  CHECK_GL_ERROR(glUniform4fv(volume_program.getUniform("light_pos"), 1, &light_pos[0]));
  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES_ADJACENCY, obj_faces.size() * 3 * 2, GL_UNSIGNED_INT, 0));
}

void ObjRenderer::draw(const glm::mat4& projection, const glm::mat4& view, const std::array<glm::vec4, NUM_LIGHTS>& light_pos, 
    const std::array<glm::mat4, NUM_LIGHTS>& depthMVP, const bool use_shadow_map, bool ambient) {
  if (!has_object) {
    return;
  }
  CHECK_GL_ERROR(glBindVertexArray(vao));
  program.activate();

  CHECK_GL_ERROR(glUniformMatrix4fv(program.getUniform("projection"), 1, GL_FALSE, &projection[0][0]));
  CHECK_GL_ERROR(glUniformMatrix4fv(program.getUniform("view"), 1, GL_FALSE, &view[0][0]));
  CHECK_GL_ERROR(glUniform1i(program.getUniform("use_shadow_map"), (int) use_shadow_map));
  CHECK_GL_ERROR(glUniform1i(program.getUniform("ambient"), (int) ambient));

  for (unsigned i = 0; i < light_pos.size(); i++) {
    std::string loc_name = "light_pos[";
    loc_name += std::to_string(i);
    loc_name += "]";
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

  CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, obj_faces.size() * 3, GL_UNSIGNED_INT, 0));
}
