#include <libs/core/keyboard.hpp>
#include <stdint.h>
#include <stddef.h>

void sys_draw_from_buffer(uint32_t* buffer, size_t width, size_t height);
bool sys_get_kb_event(Keyboard::KeyboardEvent* kb_event);
