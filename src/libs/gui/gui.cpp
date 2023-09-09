#include "gui.hpp"
#include <core/syscalls.hpp>

void draw_from_buffer(uint32_t* buffer, size_t width, size_t height) {
    if (buffer == nullptr) {
        return;
    }
    syscall(SyscallCode::DrawFromBuffer, buffer, width, height);
}

bool get_kb_event(Keyboard::KeyboardEvent* kb_event) {
    if (kb_event == nullptr) {
        return false;
    }
    return syscall(SyscallCode::GetKbEvent, kb_event);
}
