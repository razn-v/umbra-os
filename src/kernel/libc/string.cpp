#include <stddef.h>
#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* pdest = (uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void* memset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void* memmove(void* dest, const void *src, size_t n) {
    uint8_t* pdest = (uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

bool strncmp(const char* first, const char* second, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (first[i] == '\0' || second[i] == '\0') {
            return false;
        }

        if (first[i] != second[i]) {
            return false;
        }
    }

    return true;
}

bool strcmp(const char* first, const char* second) {
    size_t i = 0;

    while (first[i++] != '\0') {
        if (first[i] != second[i]) {
            return false;
        }
    }

    return true;
}

void strcpy(char* dest, const char* src) {
    size_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest;

    while (*ptr != '\0') ptr++;
    while (*src != '\0') *ptr++ = *src++;
    *ptr = '\0';

    return dest;
}

size_t strlen(const char* str) {
    size_t size = 0;
    while (*str++ != '\0') {
        size++;
    }
    return size;
}

char* strdup(const char* str) {
    size_t length = strlen(str);

    char* new_str = new char[length + 1];
    memcpy(new_str, str, length);
    new_str[length] = '\0';

    return new_str;
}
