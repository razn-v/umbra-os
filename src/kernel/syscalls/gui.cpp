#include <kernel/syscalls/gui.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/terminal.hpp>

void sys_draw_from_buffer(uint32_t* buffer, size_t width, size_t height) {
    uint32_t* framebuffer = Terminal::get_framebuffer();
    size_t framebuffer_width = Terminal::get_width();
   
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            framebuffer[y * framebuffer_width + x] = buffer[y * width + x];
        }
    }
}

bool sys_get_kb_event(Keyboard::KeyboardEvent* kb_event) {
    auto task = Scheduler::get_current_task(); 
    auto event = task->events->read();

    if (!event.is_none()) {
        *kb_event = event.unwrap();
        return true;
    }

    return false;
}
