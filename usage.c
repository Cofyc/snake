#include "usage.h"
#include "wrapper.h"

void
vreportf(const char *prefix, const char *err, va_list params)
{
    char msg[4096];

    vsnprintf(msg, sizeof(msg), err, params);
    fprintf(stderr, "%s%s\n", prefix, msg);
}

void
vwritef(int fd, const char *prefix, const char *err, va_list params)
{
    char msg[4096];

    int len = vsnprintf(msg, sizeof(msg), err, params);
    if (len > sizeof(msg))
        len = sizeof(msg);

    xwrite_in_full(fd, prefix, strlen(prefix));
    xwrite_in_full(fd, msg, len);
    xwrite_in_full(fd, "\n", 1);
}

static void (*report_routine) (const char *prefix, const char *err,
                               va_list params) = vreportf;

void
set_report_routine(void (*routine)
                        (const char *prefix, const char *err, va_list params))
{
    report_routine = routine;
}

void
usage(const char *err)
{
    fprintf(stderr, "usage: %s\n", err);
    exit(129);
}

void
die(const char *err, ...)
{
    va_list params;

    va_start(params, err);
    report_routine("fatal: ", err, params);
    va_end(params);
    exit(128);
}

int
error(const char *err, ...)
{
    va_list params;

    va_start(params, err);
    report_routine("error: ", err, params);
    va_end(params);
    return -1;
}

void
warning(const char *err, ...)
{
    va_list params;

    va_start(params, err);
    report_routine("warning: ", err, params);
    va_end(params);
}
