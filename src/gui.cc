#include "gui.h"
#include "config.h"
#include <debuggl.h>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <jpegio.h>
#include <sstream>

const double EPSILON = 0.00000001;

GUI::GUI(GLFWwindow *window)
    : window_(window) {
  glfwSetWindowUserPointer(window_, this);
  glfwSetKeyCallback(window_, KeyCallback);
  glfwSetCursorPosCallback(window_, MousePosCallback);
  glfwSetMouseButtonCallback(window_, MouseButtonCallback);
  glfwSetScrollCallback(window_, MouseScrollCallback);

  glfwGetWindowSize(window_, &window_width_, &window_height_);
  float aspect_ = static_cast<float>(window_width_) / window_height_;
  projection_matrix_ =
      glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
}

GUI::~GUI() {}

void GUI::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window_, GL_TRUE);
    return;
  }

  if (key == GLFW_KEY_R && action == GLFW_PRESS) {
    if (mods == 0) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } 
    return;
  } else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
    current_preview = (current_preview + 1) % NUM_LIGHTS;
    std::cout << "Now previewing shadow map " << current_preview << std::endl;
  } else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
    current_preview = (current_preview - 1) % NUM_LIGHTS;
    std::cout << "Now previewing shadow map " << current_preview << std::endl;
  } else if (key == GLFW_KEY_B && action == GLFW_PRESS) {
    show_preview_ = !show_preview_;
    std::cout << "Preview: " << (show_preview_ ? "shown" : "hidden") << std::endl;
  } else if (key == GLFW_KEY_V && action == GLFW_PRESS) {
    current_silhouette_idx = (current_silhouette_idx + 1) % NUM_LIGHTS;
    std::cout << "Now showing silhouette for light " << current_silhouette_idx << std::endl;
  } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
    current_silhouette_idx = (current_silhouette_idx - 1) % NUM_LIGHTS;
    std::cout << "Now showing silhouette for light " << current_silhouette_idx << std::endl;
  } else if (key == GLFW_KEY_X && action == GLFW_PRESS) {
    show_silhouettes_ = !show_silhouettes_;
    std::cout << "Silhouettes: " << (show_silhouettes_ ? "shown" : "hidden") << std::endl;
  } else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
    use_shadow_volumes_ = !use_shadow_volumes_;
    std::cout << "Shadow mode: " << (use_shadow_volumes_ ? "shadow volume" : "shadow map") << std::endl;
  }


  if (mods == 0 && captureMovement(key, action)) {
    return;
  }
}

void GUI::mousePosCallback(GLFWwindow *window, double mouse_x, double mouse_y) {
  last_x_ = current_x_;
  last_y_ = current_y_;
  current_x_ = mouse_x;
  current_y_ = window_height_ - mouse_y;
  float delta_x = current_x_ - last_x_;
  float delta_y = current_y_ - last_y_;
  if (sqrt(delta_x * delta_x + delta_y * delta_y) < 1e-15)
    return;
  glm::vec3 mouse_direction = glm::normalize(glm::vec3(delta_x, delta_y, 0.0f));
  glm::vec2 mouse_start = glm::vec2(last_x_, last_y_);
  glm::vec2 mouse_end = glm::vec2(current_x_, current_y_);
  glm::uvec4 viewport = glm::uvec4(0, 0, window_width_, window_height_);

  bool move_camera = glfwGetInputMode(window_, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;

  if (move_camera) {
    // prevent looking past straight down or up
    if (std::abs(cumulative_delta_y + delta_y) < MAX_CUMULATIVE_DELTA_Y) {
      cumulative_delta_y += delta_y;
      glm::mat3 y_rotation = glm::mat3(glm::rotate(0.01f * delta_y, glm::vec3(tangent_)));
      orientation_ = y_rotation * orientation_;
    }

    glm::mat3 x_rotation = glm::mat3(glm::rotate(-0.01f * delta_x, glm::vec3(0, 1, 0)));
    orientation_ = x_rotation * orientation_;

    tangent_ = glm::column(orientation_, 0);
    up_ = glm::column(orientation_, 1);
    look_ = glm::column(orientation_, 2);
  }
}
  
void GUI::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
    std::cout << "capturing cursor, press R to release" << std::endl;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
}

void GUI::mouseScrollCallback(double dx, double dy) {
}

void GUI::updateMotion() {
  if (pressed_keys["W"])
    eye_ += zoom_speed_ * look_;
  if (pressed_keys["S"])
    eye_ -= zoom_speed_ * look_;
  if (pressed_keys["D"])
    eye_ += pan_speed_ * tangent_;
  if (pressed_keys["A"])
    eye_ -= pan_speed_ * tangent_;
  if (pressed_keys["UP"])
    eye_ += glm::vec3(0, pan_speed_, 0);
  if (pressed_keys["DOWN"])
    eye_ -= glm::vec3(0, pan_speed_, 0);
}

void GUI::updateMatrices() {
  // Compute our view, and projection matrices.
  center_ = eye_ + camera_distance_ * look_;

  view_matrix_ = glm::lookAt(eye_, center_, up_);

  aspect_ = static_cast<float>(window_width_) / window_height_;
  projection_matrix_ =
      glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
  model_matrix_ = glm::mat4(1.0f);
}

MatrixPointers GUI::getMatrixPointers() const {
  MatrixPointers ret;
  ret.projection = &projection_matrix_[0][0];
  ret.model = &model_matrix_[0][0];
  ret.view = &view_matrix_[0][0];
  return ret;
}


void GUI::set_key_pressed(const std::string& key, int action) {
  if (action == GLFW_PRESS)
    pressed_keys[key] = true;
  if (action == GLFW_RELEASE)
    pressed_keys[key] = false;
}

bool GUI::captureMovement(int key, int action) {
  if (key == GLFW_KEY_W) {
    set_key_pressed("W", action);
  } else if (key == GLFW_KEY_S) {
    set_key_pressed("S", action);
  } else if (key == GLFW_KEY_A) {
    set_key_pressed("A", action);
  } else if (key == GLFW_KEY_D) {
    set_key_pressed("D", action);
  } else if (key == GLFW_KEY_UP) {
    set_key_pressed("UP", action);
  } else if (key == GLFW_KEY_DOWN) {
    set_key_pressed("DOWN", action);
  } else if (key == GLFW_KEY_LEFT or key == GLFW_KEY_RIGHT) {
    auto rotation = (key == GLFW_KEY_RIGHT) ? roll_speed_ : -roll_speed_;
    glm::mat3 z_rotation = glm::mat3(glm::rotate(rotation, look_));
    orientation_ = z_rotation * orientation_;

    tangent_ = glm::column(orientation_, 0);
    up_ = glm::column(orientation_, 1);
    look_ = glm::column(orientation_, 2);
  }
  return false;
}

// Delegrate to the actual GUI object.
void GUI::KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                      int mods) {
  GUI *gui = (GUI *)glfwGetWindowUserPointer(window);
  gui->keyCallback(window, key, scancode, action, mods);
}

void GUI::MousePosCallback(GLFWwindow *window, double mouse_x, double mouse_y) {
  GUI *gui = (GUI *)glfwGetWindowUserPointer(window);
  gui->mousePosCallback(window, mouse_x, mouse_y);
}

void GUI::MouseButtonCallback(GLFWwindow *window, int button, int action,
                              int mods) {
  GUI *gui = (GUI *)glfwGetWindowUserPointer(window);
  gui->mouseButtonCallback(window, button, action, mods);
}

void GUI::MouseScrollCallback(GLFWwindow *window, double dx, double dy) {
  GUI *gui = (GUI *)glfwGetWindowUserPointer(window);
  gui->mouseScrollCallback(dx, dy);
}

