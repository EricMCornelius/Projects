#ifndef GEOMETRYSHADER_H
#define GEOMETRYSHADER_H

#include "Shader.h"

#include <string>

class GeometryShader : public Shader
{
public:
    GeometryShader(const std::string& filename) : Shader(filename, GL_GEOMETRY_SHADER)
    {

    }
private:
};

#endif
