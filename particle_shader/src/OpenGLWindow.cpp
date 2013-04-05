#include "OpenGLContext.h"

#include <iostream>
#include <unordered_map>

typedef std::unordered_map<GLFWwindow*, Handler*> HandlerMap;

HandlerMap& handlers() {
  static std::unordered_map<GLFWwindow*, Handler*> registered;
  return registered;
}

void OpenGLWindow::setCallbacks() {
  handlers()[_impl] = &_handler;

  glfwSetWindowPosCallback(_impl, [](GLFWwindow*, int, int) {

  });

  glfwSetWindowSizeCallback(_impl, [](GLFWwindow* window, int width, int height) {
    std::cout << width << " x " << height << std::endl;
    auto& handler = handlers()[window];
    std::cout << handler << std::endl;
    handler->reshapeCallback(width, height);
  });

  glfwSetWindowCloseCallback(_impl, [](GLFWwindow* window) {
    glfwSetWindowShouldClose(window, true);
  });

  glfwSetWindowRefreshCallback(_impl, [](GLFWwindow*) {

  });

  glfwSetWindowFocusCallback(_impl, [](GLFWwindow*, int) {

  });

  glfwSetWindowIconifyCallback(_impl, [](GLFWwindow*, int) {

  });

  glfwSetKeyCallback(_impl, [](GLFWwindow*, int, int) {

  });

  glfwSetCharCallback(_impl, [](GLFWwindow*, unsigned int) {

  });

  glfwSetMouseButtonCallback(_impl, [](GLFWwindow*, int, int) {

  });

  glfwSetCursorPosCallback(_impl, [](GLFWwindow*, double x, double y) {
    std::cout << x << " " << y << std::endl;
  });

  glfwSetCursorEnterCallback(_impl, [](GLFWwindow*, int val) {
    if (val)
      std::cout << "Entered" << std::endl;
    else
      std::cout << "Exited" << std::endl;
  });

  glfwSetScrollCallback(_impl, [](GLFWwindow*, double, double) {

  });
}
