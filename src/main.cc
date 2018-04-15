#include <GL/glew.h>
#include <dirent.h>

#include "config.h"
#include "gui.h"
#include "render_pass.h"
#include "floor_renderer.h"
#include "preview_renderer.h"
#include "obj_renderer.h"
#include "shadow_map.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <debuggl.h>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

void read_args(int argc, char *argv[], ObjRenderer& obj_renderer) {
  const struct option opts[] = {
    { "obj", required_argument, nullptr, 'o' },
    { "pmd", required_argument, nullptr, 'p' },
  };
  
  int c;
  while ((c = getopt_long(argc, argv, "o:p:", opts, nullptr)) != -1) {
    switch (c) {
    case 'o': {
      if (!obj_renderer.load(optarg))
	std::cerr << "Failed to load OBJ " << optarg << std::endl;
      break;
    }
    case 'p': {
      /* todo */ break;
    }
    case '?': {
      std::cerr << "Usage: '" << argv[0] << " --obj <OBJ file>' or '" << argv[0] << " --pmd <PMD file>'" << std::endl;
      std::exit(-1);
    }
    }
  }
}

int main(int argc, char *argv[]) {
  GLFWwindow *window = init_glefw();
  GUI gui(window);

  const char* shadow_vert =
  #include "shaders/shadow.vert"
  ;
  const char* shadow_frag =
  #include "shaders/shadow.frag"
  ;
  ShaderProgram shadow_program;
  shadow_program
    .addVsh(shadow_vert)
    .addFsh(shadow_frag)
    .build({ "projection", "view" });

  FloorRenderer floor_renderer;
  ObjRenderer obj_renderer;
  PreviewRenderer preview_renderer;
  ShadowMap shadow_map;

  read_args(argc, argv, obj_renderer);

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
    glm::vec4 light_pos { 0, 3, 3 , 1 };
    glm::vec4 light_dir = glm::normalize(glm::vec4(0.0, -1.0, -1.0, 0));
    glm::mat4 depthMVP(1.0f);

    // capture shadows
    {
      shadow_map.begin_capture();
      auto center = light_pos + 0.5f * light_dir;
      auto view = glm::lookAt(glm::vec3(light_pos), glm::vec3(center), glm::vec3(0, 1, 0));

      // save to use in real rendering later
      depthMVP = gui.get_projection() * view;

      // render all things that cast shadows to shadow map
      shadow_program.activate();

      CHECK_GL_ERROR(glUniformMatrix4fv(shadow_program.getUniform("projection"), 1, GL_FALSE, &gui.get_projection()[0][0]));
      CHECK_GL_ERROR(glUniformMatrix4fv(shadow_program.getUniform("view"), 1, GL_FALSE, &view[0][0]));

      obj_renderer.draw_shadow();
      floor_renderer.draw_shadow();
    }

    // clean up
    CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    CHECK_GL_ERROR(glViewport(0, 0, window_width, window_height));
    CHECK_GL_ERROR(glDrawBuffer(GL_BACK));

    // draw object
    obj_renderer.draw(gui.get_projection(), gui.get_view(), light_pos);

    // draw floor
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, shadow_map.get_depth_texture()));
    floor_renderer.draw(gui.get_projection(), gui.get_view(), light_pos, depthMVP);

    // draw preview
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, shadow_map.get_depth_texture()));
    glViewport(5, 5, 640, 480);
    preview_renderer.draw(kNear, kFar);

    // Poll and swap.
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
