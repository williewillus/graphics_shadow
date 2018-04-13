#include <GL/glew.h>
#include <dirent.h>

#include "config.h"
#include "gui.h"
#include "render_pass.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <debuggl.h>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

int window_width = 1280;
int window_height = 720;
int main_view_width = 960;
int main_view_height = 720;
int preview_width = window_width - main_view_width; // 320
int preview_height = preview_width / 4 * 3;         // 320 / 4 * 3 = 240
int preview_bar_width = preview_width;
int preview_bar_height = main_view_height;
const std::string window_title = "Minecraft";

const char *vertex_shader =
#include "shaders/default.vert"
    ;

const char *blending_shader =
#include "shaders/blending.vert"
    ;

const char *geometry_shader =
#include "shaders/default.geom"
    ;

const char *fragment_shader =
#include "shaders/default.frag"
    ;

const char *floor_fragment_shader =
#include "shaders/floor.frag"
    ;

const char *bone_vertex_shader =
#include "shaders/bone.vert"
    ;

const char *bone_fragment_shader =
#include "shaders/bone.frag"
    ;

const char *cylinder_vertex_shader =
#include "shaders/cylinder.vert"
    ;

const char *cylinder_fragment_shader =
#include "shaders/cylinder.frag"
    ;

const char *preview_vertex_shader =
#include "shaders/preview.vert"
    ;

const char *preview_fragment_shader =
#include "shaders/preview.frag"
    ;


const char *shadow_vertex_shader =
#include "shaders/shadow.vert"
    ;

const char *shadow_fragment_shader =
#include "shaders/shadow.frag"
    ;


// FIXME: Add more shaders here.

void ErrorCallback(int error, const char *description) {
  std::cerr << "GLFW Error: " << description << "\n";
}

GLFWwindow *init_glefw() {
  if (!glfwInit())
    exit(EXIT_FAILURE);
  glfwSetErrorCallback(ErrorCallback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); // Disable resizing, for simplicity
  glfwWindowHint(GLFW_SAMPLES, 4);
  auto ret = glfwCreateWindow(window_width, window_height, window_title.data(),
                              nullptr, nullptr);
  CHECK_SUCCESS(ret != nullptr);
  glfwMakeContextCurrent(ret);
  glewExperimental = GL_TRUE;
  CHECK_SUCCESS(glewInit() == GLEW_OK);
  glGetError(); // clear GLEW's error for it
  glfwSwapInterval(1);
  const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte *version = glGetString(GL_VERSION);   // version as a string
  std::cout << "Renderer: " << renderer << "\n";
  std::cout << "OpenGL version supported:" << version << "\n";

  return ret;
}

int main(int argc, char *argv[]) {
  GLFWwindow *window = init_glefw();
  GUI gui(window, main_view_width, main_view_height, preview_height);

  MatrixPointers mats; // Define MatrixPointers here for lambda to capture
                       /*
                        * In the following we are going to define several lambda functions to bind
                        * Uniforms.
                        *
                        * Introduction about lambda functions:
                        *      http://en.cppreference.com/w/cpp/language/lambda
                        *      http://www.stroustrup.com/C++11FAQ.html#lambda
                        */
  /*
   * The following lambda functions are defined to bind uniforms
   */
  auto matrix_binder = [](int loc, const void *data) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat *)data);
  };
  auto vector_binder = [](int loc, const void *data) {
    glUniform4fv(loc, 1, (const GLfloat *)data);
  };
  auto vector3_binder = [](int loc, const void *data) {
    glUniform3fv(loc, 1, (const GLfloat *)data);
  };
  auto float_binder = [](int loc, const void *data) {
    glUniform1fv(loc, 1, (const GLfloat *)data);
  };
  auto int_binder = [](int loc, const void *data) {
    glUniform1iv(loc, 1, (const GLint *)data);
  };
  /*
   * The lambda functions below are used to retrieve data
   */
  auto std_model_data = [&mats]() -> const void * {
    return mats.model;
  }; // This returns point to model matrix
  glm::mat4 floor_model_matrix = glm::mat4(1.0f);
  auto floor_model_data = [&floor_model_matrix]() -> const void * {
    return &floor_model_matrix[0][0];
  }; // This return model matrix for the floor.
  auto std_view_data = [&mats]() -> const void * { return mats.view; };
  auto std_camera_data = [&gui]() -> const void * {
    return &gui.getCamera()[0];
  };
  auto std_proj_data = [&mats]() -> const void * { return mats.projection; };
  
  while (!glfwWindowShouldClose(window)) {
    // Setup some basic window stuff.
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, main_view_width, main_view_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);

    gui.updateMatrices();
    mats = gui.getMatrixPointers();

    // do everything

    // Poll and swap.
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
