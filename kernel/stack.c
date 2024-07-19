#include <page.h>
#include <size.h>
#include <stack.h>

union task_union __attribute__((aligned(PAGE_SIZE)))
__attribute__((section(".data.init_task"))) init_task_union;
