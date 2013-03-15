#ifndef SHADER
#define SHADER

#include <string>
#include <iostream>
#include <fstream>

#include <GL/glew.h>

#include "ShaderUtils.h"

using namespace std;

class Shader
{
public:
    Shader(const string& filename, unsigned int type)
    {
        ifstream file(filename);
        d_source = std::string(std::istreambuf_iterator<char>(file.rdbuf()), std::istreambuf_iterator<char>());
        file.close();

        compile(type);
        printShaderLog(d_handle);
    }

    operator unsigned int()
    {
        return d_handle;
    }
protected:
    string d_source;
    unsigned int d_handle;

    void compile(unsigned int type)
    {
        d_handle = glCreateShader(type);
        cout << "Created shader " << d_handle << endl;

        const char* sourcePtr = d_source.c_str();
        glShaderSource(d_handle, 1, &sourcePtr, 0);
        glCompileShader(d_handle);

        int compiled;
        glGetShaderiv(d_handle, GL_COMPILE_STATUS, &compiled);
        
        if(!compiled)
            cout << "Failed to compile shader with code " << compiled << endl;
    }
};

#endif
