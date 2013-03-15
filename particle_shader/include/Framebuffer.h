#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

class Framebuffer
{
public:
    Framebuffer() 
    {
        glGenFramebuffers(1, &d_handle);
    }

    Framebuffer(unsigned int handle) : d_handle(handle)
    {

    }

    unsigned int operator ()
    {
        return d_handle;
    }
private:
    unsigned int d_handle;
/*
        glGenFramebuffers(1, &frameBuffer);

        glGenRenderbuffers(1, &renderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glRenderbufferStorage(renderBuffer, GL_RGBA8, 640, 360);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBuffer);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        /*
        GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, bufs);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, drawBuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        GLenum fboStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        if(fboStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            cout << "Warning" << endl;
            switch (fboStatus)
            {
            case GL_FRAMEBUFFER_UNDEFINED:
                cout << "Undefined" << endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                cout << "Incomplete attachment" << endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                cout << "Incomplete missing attachment" << endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                cout << "Incomplete draw buffer" << endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                cout << "Incomplete read buffer" << endl;
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                cout << "Framebuffer unsupported" << endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                cout << "Incomplete multisample" << endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                cout << "Incomplete layer targets" << endl;
                break;
            default:
                cout << "Unrecognized code: " << fboStatus << endl;
            }
        }
        */
};

#endif
