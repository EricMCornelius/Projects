#ifndef SCENE_H
#define SCENE_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <armadillo>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "Program.h"
#include "VertexShader.h"
#include "FragmentShader.h"
#include "GeometryShader.h"
#include "Renderable.h"
#include "Camera.h"

float randFloat() 
{
    return ((float) rand() / (float) RAND_MAX);
}

float randFloat(float min, float max)
{
    float rand = randFloat();
    return rand * (max - min) + min;
}

class Scene : public Renderable
{
	public:
		Scene();
        ~Scene();
        void render();
        void appendBurst(int particles, float x, float y, float z, float strength, float time);
	private:
        int d_particles;

        unsigned int ProgramHandle;
        std::vector<float> verts;
        std::vector<float> colors;
        std::vector<float> velocities;
        std::vector<float> startTimes;

        bool d_animate;

        Program* program;
        FragmentShader* fragShader;
        VertexShader* vertShader;
        GeometryShader* geoShader;

        void initialize();
};

Scene::Scene() 
    : d_particles(0)
    , d_animate(true)
    , program(0)
    , fragShader(0)
    , vertShader(0)
    , geoShader(0)
{

}

void Scene::appendBurst(int particles, float x, float y, float z, float strength, float time)
{
    float colorRanges[6];
    
    for(int i = 0; i < 3; ++i)
    {
        float val = randFloat();
        if(val < 0.1)
            val = 0.1;
        else if(val > 0.9)
            val = 0.9;

        colorRanges[2*i] = val - 0.1;
        colorRanges[2*i + 1] = val + 0.1;
    }
    
    /*
    float colorRanges[] = {0.0f, 0.0f,
                          0.0f, 0.3f, 
                          0.5f, 1.0f};
                          */

    d_particles += particles;

    for (int i = 0; i < particles; ++i)
    {
        verts.push_back(x);
        verts.push_back(y);
        verts.push_back(z);

        colors.push_back(randFloat(colorRanges[0], colorRanges[1]));
        colors.push_back(randFloat(colorRanges[2], colorRanges[3]));
        colors.push_back(randFloat(colorRanges[4], colorRanges[5]));

        arma::vec3 velocity;
        velocity[0] = randFloat() - 0.5;
        velocity[1] = randFloat() - 0.5;
        velocity[2] = randFloat() - 0.5;
        velocity /= arma::norm(velocity, 2);
        velocity *= 16.0 * randFloat(0.8, 1.0) * strength;
        velocities.push_back(velocity[0]);
        velocities.push_back(velocity[1]);
        velocities.push_back(velocity[2]);

        startTimes.push_back(time);
    }
}

Scene::~Scene()
{
  if (program) {
    if (vertShader != nullptr) {
      glDetachShader(*program, *vertShader);
      glDeleteShader(*vertShader);
    }
    
    if (fragShader != nullptr) {
      glDetachShader(*program, *fragShader);
      glDeleteShader(*fragShader);
    }
    
    glDeleteProgram(*program);
  }
  // cleanup please!
}

void Scene::render()
{   
    static unsigned int colorBuffer = 0;
    static unsigned int velocityBuffer = 0;
    static unsigned int startTimeBuffer = 0;
    static VertexArray vertexArray;

    static bool initProgram = false;
    if (!initProgram) {
      fragShader = new FragmentShader("ConfettiFS.txt");
      vertShader = new VertexShader("ConfettiVS.txt");
      geoShader = new GeometryShader("ConfettiGS2.txt");

      program = new Program();

      glBindAttribLocation(*program, Program::VERTEX_ARRAY, "vertex");
      glBindAttribLocation(*program, Program::COLOR_ARRAY, "color");
      glBindAttribLocation(*program, Program::VELOCITY_ARRAY, "velocity");
      glBindAttribLocation(*program, Program::START_TIME_ARRAY, "start_time");

      glBindFragDataLocation(*program, GL_BACK, "ColorOut");

      glAttachShader(*program, *vertShader);
      glAttachShader(*program, *fragShader);
      glAttachShader(*program, *geoShader);

      program->link();
      glUseProgram(*program);

      float background[] = {0.0f, 0.0f, 0.0f, 0.0f};
      program->setBackground(background);

      program->setModelViewProjectionMatrix(theCamera.getMVP());
      initProgram = true;
    }

    static bool init = false;
    if(!init)
    {
        vertexArray.bind();

        static VertexBuffer vertexBuffer;
        static VertexBuffer colorBuffer;
        static VertexBuffer velocityBuffer;
        static VertexBuffer startTimeBuffer;

        vertexBuffer.fill(verts);
        vertexBuffer.attributeId() = Program::VERTEX_ARRAY;
        vertexBuffer.prepare();

        colorBuffer.fill(colors);
        colorBuffer.attributeId() = Program::COLOR_ARRAY;
        colorBuffer.prepare();

        velocityBuffer.fill(velocities);
        velocityBuffer.attributeId() = Program::VELOCITY_ARRAY;
        velocityBuffer.prepare();

        startTimeBuffer.fill(startTimes);
        startTimeBuffer.attributeId() = Program::START_TIME_ARRAY;
        startTimeBuffer.components() = 1;
        startTimeBuffer.prepare();

        vertexArray.unbind();

        /*
        fragShader = new FragmentShader("ConfettiFS.txt");
        vertShader = new VertexShader("ConfettiVS.txt");
        geoShader = new GeometryShader("ConfettiGS2.txt");

        program = new Program();

        glBindAttribLocation(*program, Program::VERTEX_ARRAY, "vertex");
        glBindAttribLocation(*program, Program::COLOR_ARRAY, "color");
        glBindAttribLocation(*program, Program::VELOCITY_ARRAY, "velocity");
        glBindAttribLocation(*program, Program::START_TIME_ARRAY, "start_time");

        glBindFragDataLocation(*program, GL_BACK, "ColorOut");

        glAttachShader(*program, *vertShader);
        glAttachShader(*program, *fragShader);
        glAttachShader(*program, *geoShader);

        program->link();
        glUseProgram(*program);

        float background[] = {0.0f, 0.0f, 0.0f, 0.0f};
        program->setBackground(background);

        program->setModelViewProjectionMatrix(theCamera.getMVP());
        */

        for (std::size_t i = 0; i < 50; ++i)
            appendBurst(100 * randFloat(1.0, 3.0), randFloat(-5.0, 5.0), randFloat(-5.0, 5.0), randFloat(-10.0, 10.0), randFloat(0.8, 2.0), 3.5 * randFloat());


        init = true;
    }

    static bool reverse = false;
    static float ParticleTime = 0.0f;
    static float change = 0.005f;
    ParticleTime += change;
    if(ParticleTime > 4.0)
    {
        appendBurst(100 * randFloat(1.0, 3.0), randFloat(-5.0, 5.0), randFloat(-5.0, 5.0), randFloat(-10.0, 10.0), randFloat(0.8, 2.0), 3.5 * randFloat());
        init = false;

        if(reverse)
            change = -change;
    }

    glLineWidth(2.0);

    if(reverse)
    {
        if(ParticleTime < 0.0)
            change = -change;
    }
    else
    {
        if(ParticleTime > 4.0)
            ParticleTime = 0.0;
    }
    program->setTime(ParticleTime);

    /*
    GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
    glDrawBuffers(1, bufs);
    glClear(GL_COLOR_BUFFER_BIT);
    */

    vertexArray.bind();

    glPointSize(2.0f);
    glDrawArrays(GL_POINTS, 0, d_particles);

    vertexArray.unbind();

    /*
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLenum windowBuf[] = {GL_BACK};
    glDrawBuffers(1, windowBuf);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, d_cols, d_rows, 0, 0, d_cols, d_rows, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    /*
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-1.0, -1.0, 0.1);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, -1.0, 0.1);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 1.0, 0.1);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-1.0, 1.0, 0.1);
    glEnd();
    */
}

#endif
