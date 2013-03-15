#ifndef PROGRAM_H
#define PROGRAM_H

#include <GL/glew.h>

#include <iostream>
#include "ShaderUtils.h"

using namespace std;

class Program
{
public:
    Program()
    {
        d_handle = glCreateProgram();
        cout << "Created program " << d_handle << endl;
    }

    void link()
    {
        glLinkProgram(d_handle);

        int linked;
        glGetProgramiv(d_handle, GL_LINK_STATUS, &linked);

        if(linked)
            cout << "Linked program!" << endl;
        else
        {
            cout << "Failed to link program" << endl;
            printProgramLog(d_handle);
        }

        printProgramDetails(d_handle);
    }

    void setBackground(float* background)
    {
        glUniform4fv(getUniformLocation(d_handle, "start_color"), 1, background);
    }

    void setTime(float val)
    {
        glUniform1f(getUniformLocation(d_handle, "time"), val);
    }

    void setModelViewProjectionMatrix(float* matrix)
    {
        glUniformMatrix4fv(getUniformLocation(d_handle, "mvpMatrix"), 1, GL_FALSE, matrix);
    }

    const static unsigned int VERTEX_ARRAY = 0;
    const static unsigned int COLOR_ARRAY = 1;
    const static unsigned int VELOCITY_ARRAY = 2;
    const static unsigned int START_TIME_ARRAY = 3;

    operator unsigned int()
    {
        return d_handle;
    }
private:
    unsigned int d_handle;

    static int getUniformLocation(unsigned int program, const std::string& name)
    {
        int location = 0;
    
        location = glGetUniformLocation(program, name.c_str());

        if (location == -1)
            std::cout << "No such uniform named " << name << " in program " << program << std::endl;

        return location;
    }
};

#endif
