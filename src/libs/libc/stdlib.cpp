#include "stdlib.hpp"
#include "unistd.hpp"

void exit(int status) {
    _exit(status);
}
