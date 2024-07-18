#ifndef _SIZE_H
#define _SIZE_H

#define _SIZE_H_ASSERT_SIZE(type, expected_size)                               \
  typedef char assert_size_##type[(sizeof(type) == (expected_size)) ? 1 : -1]

typedef unsigned short u16_t;

_SIZE_H_ASSERT_SIZE(u16_t, 2);

typedef unsigned long long u64_t;

_SIZE_H_ASSERT_SIZE(u64_t, 8);

#if __SIZEOF_POINTER__ == 4
typedef unsigned long u32_t;
typedef unsigned long usize_t;
typedef long size_t;
#else
typedef unsigned int u32_t;
typedef unsigned long long usize_t;
typedef long long size_t;
#endif

_SIZE_H_ASSERT_SIZE(u32_t, 4);
_SIZE_H_ASSERT_SIZE(usize_t, __SIZEOF_POINTER__);

#endif
