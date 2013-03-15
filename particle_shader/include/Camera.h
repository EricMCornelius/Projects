#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera
{
public:
    Camera() 
    {
        setPerspective(45.0f, 1.0f, 3.0f, 100.0f);
        setPosition(0.0f, 0.0f, 20.0f);
        setTarget(0.0f, 0.0f, 0.0f);
        setUp(0.0f, 1.0f, 0.0f);
    }

    void setPosition(float x, float y, float z)
    {
        d_position = glm::vec3(x, y, z);
    }

    void setTarget(float x, float y, float z)
    {
        d_target = glm::vec3(x, y, z);
    }

    void setUp(float x, float y, float z)
    {
        d_up = glm::vec3(x, y, z);
    }

    void setPerspective(float fieldOfView, float aspectRatio, float zNear, float zFar)
    {
        d_projection = glm::perspective(fieldOfView, aspectRatio, zNear, zFar);
    }

    float* getMVP()
    {
        d_modelview = glm::lookAt(d_position, d_target, d_up);
        d_mvp = d_projection * d_modelview;
        return glm::value_ptr(d_mvp);
    }
private:
    glm::vec3 d_position;
    glm::vec3 d_target;
    glm::vec3 d_up;
    glm::mat4 d_projection;
    glm::mat4 d_modelview;
    glm::mat4 d_mvp;
};

Camera theCamera;

#endif
