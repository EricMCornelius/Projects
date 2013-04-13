#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include <GL/glew.h>

#include "Bindable.h"

class VertexArray : public Bindable
{
public:
  VertexArray() {
    GLuint handle;
    glGenVertexArrays(1, &handle);
    glGenVertexArrays(1, &d_handle);
  }

  virtual ~VertexArray() { }

  bool bind() {
    if(!Bindable::bind())
      return false;

    glBindVertexArray(d_handle);
    return true;
  }

  bool unbind() {
    if(!Bindable::unbind())
      return false;

    glBindVertexArray(0);
    return true;
  }

  operator unsigned int() {
    return d_handle;
  }
private:
  unsigned int d_handle;
};

#endif
