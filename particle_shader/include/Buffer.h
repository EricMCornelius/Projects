#ifndef BUFFER_H
#define BUFFER_H

#include "Bindable.h"

#include <GL/glew.h>

class Buffer : public Bindable
{
public:
    Buffer()
    {
        glGenBuffers(1, &d_handle);
    }

    virtual ~Buffer() { }

    bool bind()
    {
        if(!Bindable::bind())
            return false;

        glBindBuffer(GL_ARRAY_BUFFER, d_handle);
        return true;
    }

    bool unbind()
    {
        if(!Bindable::unbind())
            return false;

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return true;
    }
private:
    unsigned int d_handle;
};

#endif
