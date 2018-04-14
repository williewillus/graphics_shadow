#include <GL/glew.h>
#include <dirent.h>

#include "config.h"
#include "gui.h"
#include "render_pass.h"
#include "floor_renderer.h"
#include "obj_renderer.h"

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
const std::string window_title = "Shadow Final Project";

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

bool read_args(std::vector<std::string> args, ObjRenderer& obj_renderer) {
  if (args.empty()) 
    return true;

  bool worked = false;
  if (args[0].compare("--obj") == 0 and args.size() == 2) {
    worked = obj_renderer.load(args[1]);
  }
  else if (args[0].compare("--pmd") == 0 and args.size() == 2) {
    worked = true;
    // todo 
  }
  return worked;
}

int main(int argc, char *argv[]) {
  GLFWwindow *window = init_glefw();
  GUI gui(window);

  FloorRenderer floor_renderer;
  ObjRenderer obj_renderer;

  // read in obj file
  std::vector<std::string> args;
  args.assign(argv + 1, argv + argc);

  if (!read_args(args, obj_renderer)) {
    std::cerr << "Invalid loading" << std::endl;
    std::cerr << "Usage: '" << argv[0] << " --obj <OBJ file>' or '" << argv[0] << " --pmd <PMD file>'" << std::endl;
    return -1;
  }

  MatrixPointers mats; // Define MatrixPointers here for lambda to capture
  
  while (!glfwWindowShouldClose(window)) {
    // Setup some basic window stuff.
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);

    gui.updateMotion();
    gui.updateMatrices();
    mats = gui.getMatrixPointers();

    // do everything
    floor_renderer.draw(gui.get_projection(), gui.get_view(), glm::vec4(0.0, 20.0, 0.0, 1.0));
    obj_renderer.draw(gui.get_projection(), gui.get_view(), glm::vec4(0.0, 20.0, 0.0, 1.0));

    // Poll and swap.
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
