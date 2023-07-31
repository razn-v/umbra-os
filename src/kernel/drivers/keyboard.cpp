#include <kernel/drivers/keyboard.hpp>
#include <kernel/terminal.hpp>
#include <kernel/io.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/task.hpp>

namespace Keyboard {

KbKey code_to_key(uint64_t scancode) {
    for (const KbKey key : keys) {
        if ((uint64_t)key.key_code == scancode) {
            return key;
        }
    }
    return KbKey(0, '?', '?');
}

Keyboard keyboard = {
    .buffer = 0,
    .position = 0,
    .modifiers_mask = 0
};

void handler([[gnu::unused]] Interrupt::Registers* regs) {
    uint8_t scancode = IoPort::in_port<uint8_t>(0x60);
    if (keyboard.position >= 4) {
        keyboard.clear();
    }

    keyboard.buffer = (keyboard.buffer << 8) | scancode;
    keyboard.position++;

    // Check if we have a release key
    if ((scancode >> 7) & 1) {
        scancode &= ~(1 << 7);
        KbKey key = code_to_key(scancode);

        // Remove the shift key from the modifiers
        if (key.key_code == KeyCode::LeftShift) {
            keyboard.modifiers_mask &= ~(1 << SHIFT_MASK);
        }

        keyboard.clear();
        return;
    }

    KbKey key = code_to_key(keyboard.buffer);
    if (key.key_code == KeyCode::Invalid) {
        //Terminal::printf("Invalid key\n");
    } else if (key.key_code == KeyCode::LeftShift) {
        keyboard.modifiers_mask |= 1 << SHIFT_MASK;
    } else if (key.key_code == KeyCode::CapsLock) {
        if (keyboard.caps_lock_on()) keyboard.modifiers_mask &= ~(1 << CAPS_LOCK_MASK);
        else keyboard.modifiers_mask |= 1 << CAPS_LOCK_MASK;
    } else {
        Task* current_task = Scheduler::get_current_task();
        if (current_task != nullptr) {
            char ascii = !keyboard.uppercase() ? key.ascii : key.uppercase_ascii;
            current_task->events->write(KeyboardEvent(key.key_code, ascii));
        }
        //Terminal::printf("%c", !keyboard.uppercase() ? key.ascii : key.uppercase_ascii);
    }

    keyboard.clear();
}

}
