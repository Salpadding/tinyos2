#ifndef _STRING_H
#define _STRING_H

#include <size.h>

static void memcpy(void *dst, void *src, unsigned long n) {
  unsigned char *d = dst;
  const unsigned char *s = src;
  while (n--) {
    *d++ = *s++;
  }
}

static int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = s1;
  const unsigned char *p2 = s2;

  while (n--) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }

  return 0;
}

static size_t strlen(const char *s) {
  size_t n = 0;
  while (s[n]) {
    n++;
  }
  return n;
}

static inline void memset(void *dst, char v, size_t n) {
  unsigned char *d = dst;
  while (n--) {
    *d++ = v;
  }
}

static void *memmove(void *dest, const void *src, size_t n) {
  unsigned char *d = dest;
  const unsigned char *s = src;

  if (d == s) {
    // Source and destination are the same, nothing to do
    return dest;
  }

  if (d < s) {
    // No overlap, or dest is before src, we can copy forwards
    while (n--) {
      *d++ = *s++;
    }
  } else {
    // Overlap, copy backwards
    d += n;
    s += n;
    while (n--) {
      *--d = *--s;
    }
  }

  return dest;
}

static double strtod(const char *nptr, char **endptr) { return 0; }

#endif
