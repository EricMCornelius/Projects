#include "custom_intercept.h"
#include "helper.hpp"

#include <dlfcn.h>

#include <iostream>
#include <fstream>

#include <stdio.h>

typedef int (*open_func)(const char*, int);
typedef int (*close_func)(int);
typedef ssize_t (*read_func)(int fd, void *buf, size_t count);
typedef ssize_t (*write_func)(int fd, void *buf, size_t count);

static std::ofstream outp("/var/tmp/log.txt", std::ios::app);

int open(const char *pathname, int flags)
{
  {
    mutex_helper h;
    outp << pathname << std::endl;
  }
	
  int fd;

  open_func real_open = reinterpret_cast<open_func>(dlsym(RTLD_NEXT, "open"));
  fd = real_open(pathname, flags);
  return fd;
}

int open64(const char *pathname, int flags)
{
  {
    mutex_helper h;
    outp << pathname << std::endl;
  }

  int fd;

  open_func real_open = reinterpret_cast<open_func>(dlsym(RTLD_NEXT, "open"));
  fd = real_open(pathname, flags);
  return fd;
}


int close(int fd)
{
  close_func real_close = reinterpret_cast<close_func>(dlsym(RTLD_NEXT, "close"));
  return real_close(fd);
}


ssize_t read(int fd, void *buf, size_t count)
{
  read_func real_read = reinterpret_cast<read_func>(dlsym(RTLD_NEXT, "read"));
  return real_read(fd, buf, count);
}


ssize_t write(int fd, void *buf, size_t count)
{
  write_func real_write = reinterpret_cast<write_func>(dlsym(RTLD_NEXT, "write"));
  return real_write(fd, buf, count);
}
