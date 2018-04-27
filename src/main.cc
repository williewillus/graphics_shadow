#include <GL/glew.h>
#include <dirent.h>

#include "config.h"
#include "gui.h"
#include "render_pass.h"
#include "floor_renderer.h"
#include "preview_renderer.h"
#include "obj_renderer.h"
#include "depth_map.h"
#include "ssao.h"

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
int shadow_map_height = 2048;
int shadow_map_width = 2048;
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
    { nullptr, 0, nullptr, 0 },
  };
  
  int c;
  while ((c = getopt_long(argc, argv, "o:", opts, nullptr)) != -1) {
    switch (c) {
    case 'o': {
      if (!obj_renderer.load(optarg))
	std::cerr << "Failed to load OBJ " << optarg << std::endl;
      break;
    }
    case '?': {
      std::cerr << "Usage: '" << argv[0] << " --obj <OBJ file>'" << std::endl;
      std::exit(-1);
    }
    }
  }
}

std::array<glm::vec4, NUM_LIGHTS> light_positions = {
  glm::vec4 { 0.0f, 3.0f, 3.0f, 1.0f },
  glm::vec4 { 0.0f, 3.0f, -3.0f, 1.0f },
};
static std::array<glm::vec4, NUM_LIGHTS> light_directions = {
  glm::normalize(glm::vec4(0.0, -1.0, -1.0, 0)),
  glm::normalize(glm::vec4(0.0, -1.0, 1.0, 0)),
};

static GLFWwindow *window = init_glefw();
static GUI gui(window);
static FloorRenderer floor_renderer;
static ObjRenderer obj_renderer;
static PreviewRenderer preview_renderer;

static GLuint make_volume_tex() {
  GLuint volume_tex;
  CHECK_GL_ERROR(glGenTextures(1, &volume_tex));
  CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D_ARRAY, volume_tex));
  CHECK_GL_ERROR(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, window_width, window_height, NUM_LIGHTS));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));
  return volume_tex;
}

static void render_shadow_volume(ShaderProgram& shadow_program) {
  static GLuint volume_tex = make_volume_tex();
  static std::array<TextureToRender, NUM_LIGHTS> volume_textures {
    TextureToRender(window_width, window_height, volume_tex, 0),
      TextureToRender(window_width, window_height, volume_tex, 1),
  };
  
  CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D_ARRAY, volume_tex));
  for (unsigned i = 0; i < NUM_LIGHTS; i++) {
    volume_textures.at(i).begin_capture();

    // 1. draw scene into depth buffer
    CHECK_GL_ERROR(glDepthMask(GL_TRUE));
    CHECK_GL_ERROR(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
    CHECK_GL_ERROR(glStencilMask(0));

    shadow_program.activate();
    CHECK_GL_ERROR(glUniformMatrix4fv(shadow_program.getUniform("projection"), 1, GL_FALSE, &gui.get_projection()[0][0]));
    CHECK_GL_ERROR(glUniformMatrix4fv(shadow_program.getUniform("view"), 1, GL_FALSE, &gui.get_view()[0][0]));
    obj_renderer.draw_shadow();
    floor_renderer.draw_shadow();

    // 2. draw volume into stencil buffer
    CHECK_GL_ERROR(glDepthMask(GL_FALSE));
    CHECK_GL_ERROR(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
    CHECK_GL_ERROR(glStencilMask(0xff));

    CHECK_GL_ERROR(glEnable(GL_STENCIL_TEST));
    CHECK_GL_ERROR(glEnable(GL_DEPTH_CLAMP));
    CHECK_GL_ERROR(glDisable(GL_CULL_FACE));

    // depth fail method
    CHECK_GL_ERROR(glStencilFunc(GL_ALWAYS, 0, 0xff));
    CHECK_GL_ERROR(glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP));
    CHECK_GL_ERROR(glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP));

    const auto& light_pos = light_positions.at(i);
    obj_renderer.draw_volume(gui.get_projection(), gui.get_view(), light_pos);

    // 3. draw scene for real
    CHECK_GL_ERROR(glDepthMask(GL_TRUE)); // depth writing on
    CHECK_GL_ERROR(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
    CHECK_GL_ERROR(glStencilMask(0)); // stencil writing off

    CHECK_GL_ERROR(glStencilFunc(GL_EQUAL, 0x0, 0xFF)); // draw only if stencil buf 0
    CHECK_GL_ERROR(glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_KEEP));
    floor_renderer.draw(gui.get_projection(), gui.get_view(), light_positions, std::array<glm::mat4, NUM_LIGHTS>(), false, false);
    obj_renderer.draw(gui.get_projection(), gui.get_view(), light_positions, false);
    CHECK_GL_ERROR(glDisable(GL_STENCIL_TEST));

    if (gui.show_silhouettes()) {
      obj_renderer.draw_volume(gui.get_projection(), gui.get_view(), light_positions.at(gui.get_current_light()));
    }
  }

  // draw the sum of all textures together
  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  CHECK_GL_ERROR(glViewport(0, 0, window_width, window_height));
  CHECK_GL_ERROR(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
  preview_renderer.draw_combine();
}

static GLuint make_map_tex() {
  GLuint map_depth_tex;
  CHECK_GL_ERROR(glGenTextures(1, &map_depth_tex));
  CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D_ARRAY, map_depth_tex));
  CHECK_GL_ERROR(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT16, shadow_map_width, shadow_map_height, NUM_LIGHTS));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));
  CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));
  return map_depth_tex;
}

static void render_shadow_map(ShaderProgram& shadow_program) {
  static GLuint map_depth_tex = make_map_tex();
  static std::array<DepthMap, NUM_LIGHTS> light_depth_maps {
    DepthMap(shadow_map_width, shadow_map_height, map_depth_tex, 0),
      DepthMap(shadow_map_width, shadow_map_height, map_depth_tex, 1),
  };
  
  std::array<glm::mat4, NUM_LIGHTS> depthMVP;
  CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D_ARRAY, map_depth_tex));
  shadow_program.activate();
  for (unsigned i = 0; i < NUM_LIGHTS; i++) {
    light_depth_maps.at(i).begin_capture();
    auto center = light_positions.at(i) + 0.5f * light_directions.at(i);
    auto view = glm::lookAt(glm::vec3(light_positions.at(i)), glm::vec3(center), glm::vec3(0, 1, 0));

    // save to use in real rendering later
    depthMVP.at(i) = gui.get_projection() * view;

    // render all things that cast shadows to shadow map
    CHECK_GL_ERROR(glUniformMatrix4fv(shadow_program.getUniform("projection"), 1, GL_FALSE, &gui.get_projection()[0][0]));
    CHECK_GL_ERROR(glUniformMatrix4fv(shadow_program.getUniform("view"), 1, GL_FALSE, &view[0][0]));

    obj_renderer.draw_shadow();
    floor_renderer.draw_shadow();
  }

  CHECK_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  CHECK_GL_ERROR(glViewport(0, 0, window_width, window_height));
  CHECK_GL_ERROR(glDrawBuffer(GL_BACK));

  // draw object
  obj_renderer.draw(gui.get_projection(), gui.get_view(), light_positions, false);

  // draw floor
  CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D_ARRAY, map_depth_tex));
  floor_renderer.draw(gui.get_projection(), gui.get_view(), light_positions, depthMVP, true, false);

  // draw preview
  if (gui.show_preview()) {
    CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D_ARRAY, map_depth_tex));
    glViewport(5, 5, 640, 480);
    preview_renderer.draw(kNear, kFar, gui.get_current_light());
  }
}

static void render_ssao() {
  static SSAOManager manager {window_width, window_height};
}

int main(int argc, char *argv[]) {
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

  read_args(argc, argv, obj_renderer);

  
  while (!glfwWindowShouldClose(window)) {
    // Setup some basic window stuff.
    glfwGetFramebufferSize(window, &window_width, &window_height);
    CHECK_GL_ERROR(glViewport(0, 0, window_width, window_height));
    CHECK_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
    CHECK_GL_ERROR(glEnable(GL_DEPTH_TEST));
    CHECK_GL_ERROR(glEnable(GL_MULTISAMPLE));
    CHECK_GL_ERROR(glEnable(GL_BLEND));
    CHECK_GL_ERROR(glEnable(GL_CULL_FACE));
    CHECK_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    CHECK_GL_ERROR(glDepthFunc(GL_LEQUAL));
    CHECK_GL_ERROR(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    CHECK_GL_ERROR(glCullFace(GL_BACK));

    gui.updateMotion();
    gui.updateMatrices();
    for (unsigned i = 0; i < NUM_LIGHTS; i++) {
      light_directions.at(i) = -glm::normalize(light_positions.at(i));
    }

    render_ssao(); // todo temp
    if (gui.use_shadow_volumes()) {
      render_shadow_volume(shadow_program);
    } else {
      render_shadow_map(shadow_program);
    }

    // Poll and swap.
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
