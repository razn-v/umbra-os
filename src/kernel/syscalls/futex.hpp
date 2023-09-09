#include <time.h>

int sys_futex_wait(int* pointer, int expected, const struct timespec* time);
int sys_futex_wake(int* pointer);
