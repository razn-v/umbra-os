#include <kernel/terminal.hpp>

void putchar(char ch) {
    Terminal::printf("%c", ch);
}
