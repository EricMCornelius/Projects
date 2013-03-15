#include "OpenGLContext.h"
#include "Scene.h"
#include "Renderable.h"
#include "Renderer.h"

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
  OpenGLContext ctx;
  Renderer renderer;
  OpenGLWindow window(ctx, renderer, 640, 640, "Sample Window");
  Scene scene;
  scene.appendBurst(100, 0, 0, 0, 2.0, 0.0);
  renderer.addObject(&scene);
  window.start();
}