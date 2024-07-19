#ifndef _ATOI_H
#define _ATOI_H

#include <size.h>

uint64_t atoul(const char *num);
int64_t atol(const char *num);
int64_t strtol(const char *nptr, char **endptr, int base);

#endif
