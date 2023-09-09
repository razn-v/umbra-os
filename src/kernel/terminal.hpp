#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limine/limine.h>
#include <stdarg.h>
#include <sys/types.h>

#define BLACK Rgb(118, 118, 118)
#define BLUE Rgb(59, 120, 255)
#define CYAN Rgb(97, 214, 214)
#define GREEN Rgb(22, 198, 12)
#define PURPLE Rgb(180, 0, 158)
#define RED Rgb(231, 72, 86)
#define WHITE Rgb(242, 242, 242)
#define YELLOW Rgb(249, 241, 165)
#define DARK_BLACK Rgb(12, 12, 12)
#define DARK_BLUE Rgb(0, 55, 218)
#define DARK_CYAN Rgb(58, 150, 221)
#define DARK_GREEN Rgb(19, 161, 14)
#define DARK_PURPLE Rgb(136, 23, 152)
#define DARK_RED Rgb(197, 15, 31)
#define DARK_WHITE Rgb(204, 204, 204)
#define DARK_YELLOW Rgb(193, 156, 0)

#define COLOR_MAX_LENGTH 16

// PSF 2 font face
struct PsfFont {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
};

// TODO: Make this a module loaded by the bootloader
extern char _binary_fonts_font_psf_start;
extern char _binary_fonts_font_psf_end;

union Rgb {
    struct [[gnu::packed]] {
        uint8_t blue, green, red;
    };
    uint32_t rgb;

    Rgb(uint8_t red, uint8_t green, uint8_t blue) : blue(blue), green(green), red(red) {};
};

class Terminal {
private:
    PsfFont *font;

    // Position of the last character written in the terminal
    uint8_t cursor_y, cursor_x;

    void put_char(char c, int cy, int cx, Rgb fg, Rgb bg);
    void vsprintf(char* dst, const char* format, va_list args);

    void scroll();
    void clear_screen();
 
public:
    limine_framebuffer *framebuffer;

    Terminal() = default;

    static void init(limine_framebuffer *framebuffer);
    static uint32_t* get_framebuffer();
    static uint64_t get_width();
    static uint64_t get_height();
    static void printf(const char* format, ...);
    static void puts(const char* str, ssize_t length = -1, Rgb fg = WHITE, Rgb bg = Rgb(0, 0, 0));
};
