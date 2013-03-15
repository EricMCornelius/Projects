#ifndef SHADERUTILS_H
#define SHADERUTILS_H

#include <iostream>
#include <GL/glew.h>

using namespace std;

void printShaderLog(unsigned int);
void printProgramLog(unsigned int);
void printProgramDetails(unsigned int);
void printAttachedShaders(unsigned int);
void printShaderDetails(unsigned int);

void printShaderLog(unsigned int shader)
{
    char log[1000];
    glGetShaderInfoLog(shader, 1000, NULL, log);
    cout << log << endl;
}

void printProgramLog(unsigned int program)
{
    char log[1000];
    glGetProgramInfoLog(program, 1000, NULL, log);
    cout << log << endl;
}

void printProgramDetails(unsigned int program)
{
    printAttachedShaders(program);
}

void printAttachedShaders(unsigned int program)
{
    cout << "Attached shaders:" << endl;
    unsigned int shaders[10];
    int numShaders;
    glGetAttachedShaders(program, 10, &numShaders, shaders);
    for(int i = 0; i < numShaders; ++i)
    {
        printShaderDetails(shaders[i]);
    }

    cout << "Active attributes: ";
    int paramVal;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &paramVal);
    cout << paramVal << endl;

    cout << "Active uniforms: ";
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &paramVal);
    cout << paramVal << endl;
}

void printShaderDetails(unsigned int shader)
{
    cout << "Shader " << shader << " details:" << endl;
    cout << "Source:" << endl;
    char source[1000];
    glGetShaderSource(shader, 1000, NULL, source);
    cout << source << endl;
}

#endif
