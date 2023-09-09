#include <time.h> 
#include <stdint.h>

void sys_sleep(uint64_t ms);
int sys_clock_get(int clock, time_t *secs, long *nanos);
