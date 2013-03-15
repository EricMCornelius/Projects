#ifndef FRAGMENT_SHADER_H
#define FRAGMENT_SHADER_H

#include "Shader.h"

#include <string>

class FragmentShader : public Shader
{
public:
    FragmentShader(const std::string& filename) : Shader(filename, GL_FRAGMENT_SHADER)
    {
        
    }
};

#endif
