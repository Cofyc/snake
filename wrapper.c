/**
 * Various helper functions around standard functions.
 */

#include "wrapper.h"
#include "usage.h"
#include <unistd.h>

/*
 * xread()/xwrite() are the same as read()/write(), but they automatically restart
 * opeartions with a recoverable error (EAGAIN and EINTR). They *DO NOT*
 * GUARANTEE that "len" bytes is written or read even if the operation is successful.
 *
 * If you want to do this, use *_in_full() versions.
 */
ssize_t
xread(int fd, void *buf, size_t len)
{
    ssize_t nr;

    while (1) {
        nr = read(fd, buf, len);
        if ((nr < 0) && (errno == EAGAIN || errno == EINTR))
            continue;
        return nr;
    }
}

ssize_t
xwrite(int fd, const void *buf, size_t len)
{
    ssize_t nr;

    while (1) {
        nr = write(fd, buf, len);
        if ((nr < 0) && (errno == EAGAIN || errno == EINTR))
            continue;
        return nr;
    }
}

ssize_t
xread_in_full(int fd, void *buf, size_t len)
{
    char *p = buf;
    ssize_t total = 0;

    while (len > 0) {
        ssize_t read = xread(fd, p, len);
        if (read < 0)
            return -1;
        if (read == 0)
            return total;
        len -= read;
        p += read;
        total += read;
    }

    return total;
}

ssize_t
xwrite_in_full(int fd, const void *buf, size_t len)
{
    const char *p = buf;
    ssize_t total = 0;

    while (len > 0) {
        ssize_t written = xwrite(fd, p, len);
        if (written < 0)
            return -1;
        if (!written) {
            errno = ENOSPC;
            return -1;
        }
        len -= written;
        p += written;
        total += written;
    }

    return total;
}
