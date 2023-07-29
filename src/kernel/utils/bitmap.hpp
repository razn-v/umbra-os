#include <stdint.h>
#include <stddef.h>

struct [[gnu::packed]] Bitmap {
    uint8_t* data;
    size_t size;

    void set(size_t index, bool value);
    bool get(size_t page_idx);
};
