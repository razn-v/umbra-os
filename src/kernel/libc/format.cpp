#include <kernel/libc/format.hpp>

namespace Format {

size_t int_to_str(char* buffer, int64_t integer) {
    size_t length = 0;

    bool negative = false;
    if (integer < 0) {
        negative = true;
        integer = -integer;
    }

    // Convert each digit from right to left into a char
    do {
        int64_t digit = integer % 10;
        buffer[length++] = digit + '0';
        integer /= 10;
    } while (integer != 0);

    if (negative) {
        buffer[length++] = '-';
    }

    // Reverse the buffer in place
    for (size_t i = 0; i < length / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[length - i - 1];
        buffer[length - i - 1] = temp;
    }

    return length;
}

size_t uint_to_hex(char* buffer, uint64_t integer) {
    const char* hex_chars = "0123456789abcdef";
    size_t length = 0;

    if (integer < 0) {
        integer = -integer;
    }

    // Convert each digit from right to left into a hexadecimal char
    do {
        uint64_t remainder = integer % 16;
        buffer[length++] = hex_chars[remainder];
        integer /= 16;
    } while (integer > 0);

    // Reverse the buffer in place
    for (size_t i = 0; i < length / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[length - i - 1];
        buffer[length - i - 1] = temp;
    }

    return length;
}

}
