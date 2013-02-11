#ifndef WRAPPER_H
#define WRAPPER_H

#include "compat.h"

ssize_t xread(int fd, void *buf, size_t len);
ssize_t xwrite(int fd, const void *buf, size_t len);
ssize_t xread_in_full(int fd, void *buf, size_t count);
ssize_t xwrite_in_full(int fd, const void *buf, size_t count);
static inline ssize_t
xwrite_str_in_full(int fd, const char *str)
{
    return xwrite_in_full(fd, str, strlen(str));
}

#endif
