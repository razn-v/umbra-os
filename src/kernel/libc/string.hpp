#include <stddef.h>

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);

bool strncmp(const char* first, const char* second, size_t length);
bool strcmp(const char* first, const char* second);
void strcpy(char* dest, const char *src);
char* strcat(char* dest, const char* src);
size_t strlen(const char* str);
char* strdup(const char* str);
