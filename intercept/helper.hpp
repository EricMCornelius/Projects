#ifndef HELPER_HPP
#define HELPER_HPP

struct impl;

struct mutex_helper {
  mutex_helper();
  ~mutex_helper();

  impl* _impl;
};

#endif

