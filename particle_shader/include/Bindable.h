#ifndef BINDABLE_H
#define BINDABLE_H

class Bindable
{
public:
    Bindable() : d_bound(false)
    {

    }

    virtual ~Bindable() { }

    virtual bool bind()
    {
        if(!d_bound)
        {
            d_bound = true;
            return true;
        }

        return false;
    }

    virtual bool unbind()
    {
        if(d_bound)
        {
            d_bound = false;
            return false;
        }
    }

    virtual bool boundState()
    {
        return d_bound;
    }
private:
    bool d_bound;
};

#endif
