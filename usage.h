#ifndef USAGE_H
#define USAGE_H

#include "compat.h"

typedef void (*routine) (const char *prefix, const char *err, va_list params);

void vreportf(const char *prefix, const char *err, va_list params);
void vwritef(int fd, const char *prefix, const char *err, va_list params);
void set_report_routine(routine routine);
void die(const char *err, ...);
void usage(const char *err);
int error(const char *err, ...);
void warning(const char *err, ...);

#endif
