#include <stdint.h>
#include <keyboard.hpp>

void exit(int status);
void sleep(uint64_t ms);
KeyboardEvent poll_event();
void putchar(char ch);
