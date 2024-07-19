#ifndef _STACK_H
#define _STACK_H

#include <linux/list.h>
#include <page.h>

#ifndef STACK_SIZE
#define STACK_SIZE (PAGE_SIZE << 3)
#endif

struct task_struct {};

union task_union {
  struct task_struct task;
  unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
};

extern union task_union init_task_union;

#endif
