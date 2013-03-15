#ifndef VERTEX_SHADER_H
#define VERTEX_SHADER_H

#include "Shader.h"

#include <string>

class VertexShader : public Shader
{
public:
    VertexShader(const std::string& filename) : Shader(filename, GL_VERTEX_SHADER)
    {

    }
};

#endif
