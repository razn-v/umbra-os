#include "stdlib.hpp"

using Function = void (*)();

extern "C" Function init_array_start;
extern "C" Function init_array_end;

extern "C" Function fini_array_start;
extern "C" Function fini_array_end;

extern int main();

extern "C" void pre_main() {
    for (Function* ctor = &init_array_start; ctor != &init_array_end; ctor++) {
        (*ctor)();
    }

    int status = main();
    
    for (Function* ctor = &fini_array_start; ctor != &fini_array_end; ctor++) {
        (*ctor)();
    }   

    exit(status);
}