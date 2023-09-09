#pragma once

#include <stdint.h>
#include <kernel/int.hpp>
#include <libs/core/keyboard.hpp>

namespace Keyboard {

struct Key {
    // We only support scan code set 1 for now, so no kernel-specific scancode
    KeyCode key_code;
    // ASCII representation of the key
    char ascii;
    // ASCII representation of the key when Shift is pressed or Caps Lock is enabled
    char uppercase_ascii;

    constexpr Key(uint64_t scancode, char ascii, char uppercase_ascii) :
        key_code((KeyCode)scancode), ascii(ascii), uppercase_ascii(uppercase_ascii) {};
};

#define KEY(id, scancode, ascii, uppercase_ascii) Key(scancode, ascii, uppercase_ascii),
const Key keys[] {
    KEYS
};
#undef KEY

Key code_to_key(uint64_t scancode);

#define CTRL_MASK      0
#define ALT_MASK       1
#define SHIFT_MASK     2
#define CAPS_LOCK_MASK 3

struct Keyboard {
    // Bitfield storing scancodes received before being processed (1 per byte)
    uint64_t buffer;
    // Position in the buffer
    uint8_t position;
    // Indicates if CTRL, ALT or SHIFT are pressed
    uint8_t modifiers_mask;

    void clear() {
        this->buffer = 0;
        this->position = 0;
    }

    bool ctrl_pressed() {
        return ((this->modifiers_mask >> CTRL_MASK) & 1) == 1;
    }

    bool alt_pressed() {
        return ((this->modifiers_mask >> ALT_MASK) & 1) == 1;
    }

    bool shift_pressed() {
        return ((this->modifiers_mask >> SHIFT_MASK) & 1) == 1;
    }

    bool caps_lock_on() {
        return ((this->modifiers_mask >> CAPS_LOCK_MASK) & 1) == 1;
    }

    bool uppercase() {
        return this->shift_pressed() || this->caps_lock_on();
    }
};

void handler(Interrupt::Registers* regs);

}
