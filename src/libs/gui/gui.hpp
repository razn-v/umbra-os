#include <stdint.h>
#include <stddef.h>
#include <core/keyboard.hpp>

void draw_from_buffer(uint32_t* buffer, size_t width, size_t height);
bool get_kb_event(Keyboard::KeyboardEvent* kb_event);
