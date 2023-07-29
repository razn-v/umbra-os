#include <kernel/utils/bitmap.hpp>

void Bitmap::set(size_t page_idx, bool value) {
    size_t byte_index = page_idx / 8;
    size_t bit_index = page_idx % 8;

    if (value) this->data[byte_index] |= (1 << bit_index);
    else       this->data[byte_index] &= ~(1 << bit_index);
}

bool Bitmap::get(size_t page_idx) {
    size_t byte_index = page_idx / 8;
    size_t bit_index = page_idx % 8;
    return (this->data[byte_index] >> bit_index) & 1;
}
