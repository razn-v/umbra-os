#include <kernel/drivers/tty.hpp>
#include <kernel/terminal.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/int.hpp>

size_t Tty::read([[gnu::unused]] Vfs::Node* node, void* buffer, [[gnu::unused]] size_t offset, 
        size_t length) {
    size_t read = 0;
    while (true) {
        if (read == length) {
            break;
        }

        // Wait for a keyboard input
        auto kb_events = Scheduler::get_current_task()->events;
        Scheduler::block_on([kb_events]() { return !kb_events->is_empty(); });

        Keyboard::KeyboardEvent event = kb_events->read().unwrap();
        if (!event.pressed) continue;

        ((uint8_t*)buffer)[read] = event.ascii; 
        Terminal::printf("%c", event.ascii);

        read++;
        if (event.ascii == '\n') {
            return read;
        }
    }

    return length;
}

size_t Tty::write(Vfs::Node* node, const void* buffer, [[gnu::unused]] size_t offset, 
        size_t length) {
    if (node->type != Vfs::NodeType::File) {
        return 0;
    }

    // We don't buffer stdout, no need to write into `node->file_data` here
    Terminal::puts((const char*)buffer, length); 

    return length;
}
