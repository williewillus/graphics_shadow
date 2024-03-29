#ifndef SKINNING_GUI_H
#define SKINNING_GUI_H

#include "tictoc.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>
#include <unordered_map>

struct Mesh;

/*
 * Hint: call glUniformMatrix4fv on thest pointers
 */
struct MatrixPointers {
  const float *projection, *model, *view;
};

enum Mode {
  MAP,
  VOLUME,
  SSAO,
};

class GUI {
public:
  GUI(GLFWwindow *window);
  ~GUI();

  float scroll = 0.0f;

  void assignMesh(Mesh *);

  void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  void mousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y);
  void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  void mouseScrollCallback(double dx, double dy);
  void updateMatrices();
  MatrixPointers getMatrixPointers() const;

  static void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  static void MousePosCallback(GLFWwindow *window, double mouse_x,
                               double mouse_y);
  static void MouseButtonCallback(GLFWwindow *window, int button, int action,
                                  int mods);
  static void MouseScrollCallback(GLFWwindow *window, double dx, double dy);

  glm::vec3 getCenter() const { return center_; }
  const glm::vec3 &getCamera() const { return eye_; }

  float getCamDist() const { return camera_distance_; }
  void setCamDist(float f) { camera_distance_ = f; }

  const glm::mat4& get_view()       const { return view_matrix_; }
  const glm::mat4& get_projection() const { return projection_matrix_; }
  unsigned get_current_light() const { return current_light; }
  bool debug_mode() const { return debug_mode_; }
  Mode current_mode() const { return current_mode_; }

  void updateMotion();
  
private:
  GLFWwindow *window_;

  int window_width_, window_height_;

  unsigned current_light = 0;
  bool debug_mode_ = false;
  bool drag_state_ = false;
  Mode current_mode_ = VOLUME;
  int current_button_ = -1;
  float roll_speed_ = M_PI / 64.0f;
  float last_x_ = 0.0f, last_y_ = 0.0f, current_x_ = 0.0f, current_y_ = 0.0f;
  float last_lpress_x_ = -1.0f, last_lpress_y_ = -1.0f;
  float camera_distance_ = 3.0;
  float pan_speed_ = 0.01f;
  float rotation_speed_ = 0.02f;
  float zoom_speed_ = 0.01f;
  float scroll_speed_ = 5.0f;
  float aspect_;

  float cumulative_delta_y = 0;
  static constexpr float MAX_CUMULATIVE_DELTA_Y = 155.0f;

  glm::vec3 eye_ = glm::vec3(0.0f, 0.2f, camera_distance_);
  glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 look_ = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 tangent_ = glm::cross(look_, up_);
  glm::vec3 center_ = eye_ - camera_distance_ * look_;
  glm::mat3 orientation_ = glm::mat3(tangent_, up_, look_);

  glm::mat4 view_matrix_ = glm::lookAt(eye_, center_, up_);
  glm::mat4 projection_matrix_;
  glm::mat4 model_matrix_ = glm::mat4(1.0f);

  std::unordered_map<std::string, bool> pressed_keys;

  void set_key_pressed(const std::string& key, int action);
  bool captureMovement(int key, int action);
};

#endif
