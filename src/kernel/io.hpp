#include <stdint.h>

namespace IoPort {

template <typename T>
static void out_port(uint16_t port, T value) {
    if constexpr (sizeof(T) == 1) {
        asm volatile("outb %1, %0" : : "d"(port), "a"(value));
    } else if constexpr (sizeof(T) == 2) {
        asm volatile("outw %1, %0" : : "d"(port), "a"(value));
    } else if constexpr (sizeof(T) == 4) {
        asm volatile("outl %1, %0" : : "d"(port), "a"(value));
    }
}

template <typename T>
static T in_port(uint16_t port) {
    T value;

    if constexpr (sizeof(T) == 1) {
        asm volatile("inb %1, %0" : "=a"(value) : "dN"(port));
    } else if constexpr (sizeof(T) == 2) {
        asm volatile("inw %1, %0" : "=a"(value) : "dN"(port));
    } else if constexpr (sizeof(T) == 4) {
        asm volatile("inl %1, %0" : "=a"(value) : "dN"(port));
    }

    return value;
}

}
