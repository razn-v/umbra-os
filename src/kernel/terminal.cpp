#include <kernel/terminal.hpp>
#include <kernel/libc/format.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/lock.hpp>

static Terminal terminal;
static Spinlock terminal_lock;

void Terminal::init(limine_framebuffer *framebuffer) {
    terminal.framebuffer = framebuffer;
    terminal.font = (PsfFont*) &_binary_fonts_font_psf_start;
}

void Terminal::clear_screen() {
    for (uint32_t i = 0; i < this->framebuffer->height * this->framebuffer->width; i++) {
        ((uint32_t*)this->framebuffer->address)[i] = 0;
    }
}

void Terminal::scroll() {
    uint32_t line_size = this->framebuffer->width * sizeof(uint32_t);
    uint32_t buffer_size = (this->framebuffer->height - 1) * line_size;
    uint32_t* dest = (uint32_t*)this->framebuffer->address;
    uint32_t* src = (uint32_t*)((uintptr_t)this->framebuffer->address + 
            this->framebuffer->width * this->font->height * sizeof(uint32_t));

    memcpy(dest, src, buffer_size);
}

void Terminal::put_char(char c, int cy, int cx, Rgb fg, Rgb bg) {
    uint8_t* glyph = &((uint8_t*)this->font)[32 + c * this->font->bytesperglyph];

    for (uint32_t i = 0; i < this->font->height; i++) {
        for (uint32_t j = 0; j < this->font->width; j++) {
            uint32_t pixelIndex = cx * this->font->width + j + this->framebuffer->width *
                (i + cy * this->font->height);

            uint32_t pixelColor = (glyph[i] >> (this->font->width-j)) & 1
                ? fg.rgb
                : bg.rgb;

            ((uint32_t*)this->framebuffer->address)[pixelIndex] = pixelColor;
        }
    }
}

void Terminal::printf(const char* format, ...) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    va_list args;
    va_start(args, format);
    terminal.vsprintf(buffer, format, args);
    va_end(args);

    terminal_lock.acquire();
    terminal.puts(buffer);
    terminal_lock.release();
}

void Terminal::puts(const char* str, Rgb fg, Rgb bg) {
    uint64_t y = this->cursor_y;
    uint64_t x = this->cursor_x;

    Rgb last_color = fg;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            y++;
            x = 0;
            continue;
        }

        // Color specification
        if (str[i] == '{') {
            char code[COLOR_MAX_LENGTH] = { 0 };
            size_t tmp = 0;
            size_t saved_idx = i;

            i++;
            while (str[i] != '}') {
                code[tmp++] = str[i];
                i++;
            }

            if (strcmp(code, "red")) {
                last_color = RED;
                continue;
            } else if (strcmp(code, "green")) {
                last_color = GREEN;
                continue;
            } else if (strcmp(code, "white")) {
                last_color = WHITE;
                continue;
            } else if (strcmp(code, "yellow")) {
                last_color = YELLOW;
                continue;
            } else {
                i = saved_idx;
            }
        }

        if (x >= this->framebuffer->width / this->font->width) {
            x = 0;
            y++;
        }
        if (y >= this->framebuffer->height / this->font->height) {
            this->scroll();
            y--;
            //this->clear_screen();
            //y = 0;
        }

        this->put_char(str[i], y, x, last_color, bg);
        x++;
    }

    this->cursor_y = y;
    this->cursor_x = x;
}

// TODO: Add specifiers for background/foreground color
void Terminal::vsprintf(char* buffer, const char* format, va_list args) {
    bool format_specifier = false;
    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%') {
            format_specifier = true;
            continue;
        }

        if (!format_specifier) {
            *buffer = format[i];
            buffer++;
            continue;
        }

        switch (format[i]) {
            // Character
            case 'c': {
                // Using `char` instead of `int` here gives us a warning
                char character = va_arg(args, int);
                *buffer = character;
                buffer++;
                break;
            }
            // Signed integer
            case 'd': {
                int64_t integer = va_arg(args, int64_t);
                size_t size = Format::int_to_str(buffer, integer);
                buffer += size;
                break;
            }
            // String
            case 's': {
                char *str = va_arg(args, char*);
                while (*str != '\0') {
                    *buffer = *str;
                    buffer++;
                    str++;
                }
                break;
            }
            // Hexadecimal
            case 'x': {
                uint64_t integer = va_arg(args, uint64_t);
                size_t size = Format::uint_to_hex(buffer, integer);
                buffer += size;
                break;
            }
        }

        format_specifier = false;
    }
}
