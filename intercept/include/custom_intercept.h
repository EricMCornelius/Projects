#include <sys/types.h>

extern "C" {

void * __dso_handle = 0;

int open(const char *pathname, int flags);

int open64(const char *pathname, int flags);

int close(int fd);

ssize_t read(int fd, void *buf, size_t count);

ssize_t write(int fd, void *buf, size_t count);

}
