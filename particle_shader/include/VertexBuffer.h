#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "Buffer.h"

#include <GL/glew.h>
#include <vector>

class VertexBuffer : public Buffer
{
public:
    VertexBuffer()
        : d_attributeId(0)
        , d_components(3)
        , d_dataType(GL_FLOAT)
        , d_normalized(false)
        , d_stride(0)
        , d_offset(0)
    {

    }

    virtual ~VertexBuffer() { }

    template <typename T>
    void fill(const std::vector<T>& data, unsigned int mode = GL_STATIC_DRAW)
    {
        Buffer::bind();
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), &data[0], mode);
        Buffer::unbind();
    }

    void prepare()
    {
        Buffer::bind();
        glVertexAttribPointer(d_attributeId, d_components, d_dataType, d_normalized, d_stride, (const void*) d_offset);
        glEnableVertexAttribArray(d_attributeId);
        Buffer::unbind();
    }

    unsigned int& attributeId()
    {
        return d_attributeId;
    }

    unsigned int& components()
    {
        return d_components;
    }

    unsigned int& dataType()
    {
        return d_dataType;
    }

    bool& normalized()
    {
        return d_normalized;
    }

    unsigned int& stride()
    {
        return d_stride;
    }

    unsigned int& offset()
    {
        return d_offset;
    }
private:
    unsigned int d_attributeId;
    unsigned int d_components;
    unsigned int d_dataType;
    bool         d_normalized;
    unsigned int d_stride;
    unsigned int d_offset;
};

#endif
