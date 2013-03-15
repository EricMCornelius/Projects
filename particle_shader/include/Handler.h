#ifndef HANDLER_H
#define HANDLER_H

class Handler
{
public:
    virtual void displayCallback() = 0;
    virtual void reshapeCallback(int width, int height) = 0;
    virtual void keyDownCallback(unsigned char key, int x, int y) = 0;
    virtual void keyUpCallback(unsigned char key, int x, int y) = 0;
    virtual void mouseDownCallback(int button, int x, int y) = 0;
    virtual void mouseUpCallback(int button, int x, int y) = 0;
    virtual void idleCallback() = 0;
};

#endif
