#include <GL/glew.h>
#include <dirent.h>

#include "config.h"
#include "gui.h"
#include "render_pass.h"
#include "floor_renderer.h"

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

  FloorRenderer floor_renderer;
  
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
    floor_renderer.draw();

    // Poll and swap.
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
