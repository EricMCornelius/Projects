#ifndef OPENGLCONTEXT_H
#define OPENGLCONTEXT_H

#include <GL/glew.h>
#include <GL/glfw3.h>

#include "Handler.h"

#include <string>
#include <iostream>
#include <stdexcept>

void sizeCallback(GLFWwindow* window, int width, int height);

class OpenGLContext {
public:
  OpenGLContext() { 
    if (!glfwInit())
      throw std::runtime_error("Unable to initialize glfw");
  }

  void setup() {
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  }
};

class OpenGLWindow
{
public:
  OpenGLWindow(OpenGLContext& ctx, Handler& handler, std::size_t width, std::size_t height, const std::string& name)
    : _ctx(ctx), _handler(handler), _width(width), _height(height), _name(name) {

    ctx.setup();
    _impl = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (_impl == nullptr)
      throw std::runtime_error("Unable to create glfw window");
   
    std::cout << glfwGetWindowParam(_impl, GLFW_CONTEXT_VERSION_MAJOR) << " " << glfwGetWindowParam(_impl, GLFW_CONTEXT_VERSION_MINOR) << std::endl;

    glfwMakeContextCurrent(_impl);

    glewExperimental = GL_TRUE;
    auto err = glewInit();
    if (GLEW_OK != err) {
      std::cout << glewGetErrorString(err) << std::endl;
      throw std::runtime_error("Unable to initialize glew");
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    setCallbacks();
  }

  ~OpenGLWindow() {
    glfwDestroyWindow(_impl);
  }

  void setCallbacks(); 

  void start() {
    bool stop = false;
    while(!stop) {
      glfwPollEvents();
      stop = glfwWindowShouldClose(_impl);
      _handler.displayCallback();
      glfwSwapBuffers(_impl);
    }
  }

  /*
  static void Mouse(int button, int state, int x, int y) {
    if(state == 0)
      d_handler->mouseDownCallback(button, x, y);
    else if(state == 1)
      d_handler->mouseUpCallback(button, x, y);
  }
  */

private:
  OpenGLContext& _ctx; 
  Handler& _handler;
  std::size_t _width;
  std::size_t _height;
  const std::string _name;

  GLFWwindow* _impl;
};

#endif
