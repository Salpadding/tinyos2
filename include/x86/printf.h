#ifndef _X86_PRINTF_H
#define _X86_PRINTF_H
#include <size.h>

#define va_list __builtin_va_list
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define fallthrough

#define x86_print_isdigit(c) (c >= '0' && c <= '9')

static size_t x86_print_strnlen(const char *s, size_t maxlen) {
  const char *es = s;
  while (*es && maxlen) {
    es++;
    maxlen--;
  }

  return (es - s);
}

static int x86_print_skip_atoi(const char **s) {
  int i = 0;

  while (x86_print_isdigit(**s))
    i = i * 10 + *((*s)++) - '0';
  return i;
}

#define ZEROPAD 1  /* pad with zero */
#define SIGN 2     /* unsigned/signed long */
#define PLUS 4     /* show plus */
#define SPACE 8    /* space if plus */
#define LEFT 16    /* left justified */
#define SMALL 32   /* Must be 32 == 0x20 */
#define SPECIAL 64 /* 0x */

#define x86_print__do_div(n, base)                                             \
  ({                                                                           \
    int __res;                                                                 \
    __res = ((unsigned long)n) % (unsigned)base;                               \
    n = ((unsigned long)n) / (unsigned)base;                                   \
    __res;                                                                     \
  })

static char *x86_print_number(char *str, long num, int base, int size,
                              int precision, int type) {
  /* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
  static const char digits[16] =
      "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

  char tmp[66];
  char c, sign, locase;
  int i;

  /* locase = 0 or 0x20. ORing digits or letters with 'locase'
   * produces same digits or (maybe lowercased) letters */
  locase = (type & SMALL);
  if (type & LEFT)
    type &= ~ZEROPAD;
  if (base < 2 || base > 16)
    return (void *)0;
  c = (type & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (type & SIGN) {
    if (num < 0) {
      sign = '-';
      num = -num;
      size--;
    } else if (type & PLUS) {
      sign = '+';
      size--;
    } else if (type & SPACE) {
      sign = ' ';
      size--;
    }
  }
  if (type & SPECIAL) {
    if (base == 16)
      size -= 2;
    else if (base == 8)
      size--;
  }
  i = 0;
  if (num == 0)
    tmp[i++] = '0';
  else
    while (num != 0)
      tmp[i++] = (digits[x86_print__do_div(num, base)] | locase);
  if (i > precision)
    precision = i;
  size -= precision;
  if (!(type & (ZEROPAD + LEFT)))
    while (size-- > 0)
      *str++ = ' ';
  if (sign)
    *str++ = sign;
  if (type & SPECIAL) {
    if (base == 8)
      *str++ = '0';
    else if (base == 16) {
      *str++ = '0';
      *str++ = ('X' | locase);
    }
  }
  if (!(type & LEFT))
    while (size-- > 0)
      *str++ = c;
  while (i < precision--)
    *str++ = '0';
  while (i-- > 0)
    *str++ = tmp[i];
  while (size-- > 0)
    *str++ = ' ';
  return str;
}

static int x86_print_vsprintf(char *buf, const char *fmt, va_list args) {
  int len;
  unsigned long num;
  int i, base;
  char *str;
  const char *s;

  int flags; /* flags to number() */

  int field_width; /* width of output field */
  int precision;   /* min. # of digits for integers; max
                      number of chars for from string */
  int qualifier;   /* 'h', 'l', or 'L' for integer fields */

  for (str = buf; *fmt; ++fmt) {
    if (*fmt != '%') {
      *str++ = *fmt;
      continue;
    }

    /* process flags */
    flags = 0;
  repeat:
    ++fmt; /* this also skips first '%' */
    switch (*fmt) {
    case '-':
      flags |= LEFT;
      goto repeat;
    case '+':
      flags |= PLUS;
      goto repeat;
    case ' ':
      flags |= SPACE;
      goto repeat;
    case '#':
      flags |= SPECIAL;
      goto repeat;
    case '0':
      flags |= ZEROPAD;
      goto repeat;
    }

    /* get field width */
    field_width = -1;
    if (x86_print_isdigit(*fmt))
      field_width = x86_print_skip_atoi(&fmt);
    else if (*fmt == '*') {
      ++fmt;
      /* it's the next argument */
      field_width = va_arg(args, int);
      if (field_width < 0) {
        field_width = -field_width;
        flags |= LEFT;
      }
    }

    /* get the precision */
    precision = -1;
    if (*fmt == '.') {
      ++fmt;
      if (x86_print_isdigit(*fmt))
        precision = x86_print_skip_atoi(&fmt);
      else if (*fmt == '*') {
        ++fmt;
        /* it's the next argument */
        precision = va_arg(args, int);
      }
      if (precision < 0)
        precision = 0;
    }

    /* get the conversion qualifier */
    qualifier = -1;
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
      qualifier = *fmt;
      ++fmt;
    }

    /* default base */
    base = 10;

    switch (*fmt) {
    case 'c':
      if (!(flags & LEFT))
        while (--field_width > 0)
          *str++ = ' ';
      *str++ = (unsigned char)va_arg(args, int);
      while (--field_width > 0)
        *str++ = ' ';
      continue;

    case 's':
      s = va_arg(args, char *);
      len = x86_print_strnlen(s, precision);

      if (!(flags & LEFT))
        while (len < field_width--)
          *str++ = ' ';
      for (i = 0; i < len; ++i)
        *str++ = *s++;
      while (len < field_width--)
        *str++ = ' ';
      continue;

    case 'p':
      if (field_width == -1) {
        field_width = 2 * sizeof(void *);
        flags |= ZEROPAD;
      }
      str = x86_print_number(str, (unsigned long)va_arg(args, void *), 16,
                             field_width, precision, flags);
      continue;

    case 'n':
      if (qualifier == 'l') {
        long *ip = va_arg(args, long *);
        *ip = (str - buf);
      } else {
        int *ip = va_arg(args, int *);
        *ip = (str - buf);
      }
      continue;

    case '%':
      *str++ = '%';
      continue;

      /* integer number formats - set up the flags and "break" */
    case 'o':
      base = 8;
      break;

    case 'x':
      flags |= SMALL;
      fallthrough;
    case 'X':
      base = 16;
      break;

    case 'd':
    case 'i':
      flags |= SIGN;
      break;

    case 'u':
      break;

    default:
      *str++ = '%';
      if (*fmt)
        *str++ = *fmt;
      else
        --fmt;
      continue;
    }
    if (qualifier == 'l')
      num = va_arg(args, unsigned long);
    else if (qualifier == 'h') {
      num = (unsigned short)va_arg(args, int);
      if (flags & SIGN)
        num = (short)num;
    } else if (flags & SIGN)
      num = va_arg(args, int);
    else
      num = va_arg(args, unsigned int);
    str = x86_print_number(str, num, base, field_width, precision, flags);
  }
  *str = '\0';
  return str - buf;
}

static int x86_print_sprintf(char *buf, const char *fmt, ...) {
  va_list args;
  int i;

  va_start(args, fmt);
  i = x86_print_vsprintf(buf, fmt, args);
  va_end(args);
  return i;
}

#endif
