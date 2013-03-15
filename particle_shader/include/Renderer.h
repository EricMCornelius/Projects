#ifndef RENDERER_H
#define RENDERER_H

#include "Renderable.h"
#include "Handler.h"
#include "Camera.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include <list>

#include <boost/thread.hpp>

class Renderer : public Handler
{
public:
    Renderer()
    {
        
    }

    void addObject(Renderable* obj)
    {
        objs.push_back(obj);
    }

    void displayCallback()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(auto itr = objs.begin(); itr != objs.end(); ++itr)
            (*itr)->render();

        //boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    }

    void reshapeCallback(int width, int height)
    {
        float aspect = (float) width / (float) height;
        glViewport(0, 0, width, height);
        theCamera.setPerspective(90, aspect, 3, 100);
    }

    void keyDownCallback(unsigned char key, int x, int y)
    {
        cout << key << " " << x << " " << y << endl;
        switch(key)
        {
            case '+':
                break;
            case '-':
                break;
            default:
                cout << "Unrecognized key" << endl;
        }
    }

    void keyUpCallback(unsigned char key, int x, int y)
    {
        /*
        cout << key << " " << x << " " << y << endl;
        switch(key)
        {
            default:
                cout << "Unrecognized key" << endl;
        }
        */
    }

    void mouseDownCallback(int button, int x, int y)
    {
        //cout << button << " " << x << " " << y << endl;
        switch(button)
        {
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
        }
    }

    void mouseUpCallback(int button, int x, int y)
    {
        //cout << button << " " << x << " " << y << endl;
        switch(button)
        {
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
        }
    }

    void idleCallback()
    {

    }
private:
    std::list<Renderable*> objs;
};

#endif
