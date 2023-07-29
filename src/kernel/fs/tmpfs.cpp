#include <kernel/fs/tmpfs.hpp>
#include <kernel/libc/string.hpp>

size_t Tmpfs::read(Vfs::Node* node, void* buffer, size_t offset, size_t length) {
    memcpy(buffer, node->file_data + offset, length);
    return length;
}

size_t Tmpfs::write(Vfs::Node* node, void* buffer, size_t offset, size_t length) {
    memcpy(node->file_data + offset, buffer, length);
    return length;
}