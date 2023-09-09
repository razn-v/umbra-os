#include <kernel/fs/initramfs.hpp>
#include <kernel/fs/vfs.hpp>
#include <kernel/requests.hpp>
#include <kernel/terminal.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/memory/pmm.hpp>

namespace Initramfs {

uint64_t oct_to_dec(const char* str) {
    uint64_t res = 0;

    while (*str != '\0') {
        res *= 8;
        res += *str - '0'; 
        str++;
    }

    return res;
}

void init() {
    struct limine_file** modules = module_request.response->modules;
    Initramfs::UstarHeader* archive = (Initramfs::UstarHeader*)modules[0]->address;

    while (strncmp(archive->magic, "ustar", 5)) {
        uint64_t size = oct_to_dec(archive->size);

        switch (archive->type_flag) {
            case USTAR_TYPE_NORMAL: {
                Vfs::Node* node = Vfs::create(archive->name, Vfs::NodeType::File);
                node->file_data = new uint8_t[size];
                node->file_size = size;
                memcpy(node->file_data, (void*)((uintptr_t)archive + 512), size);
                break;
            }
            case USTAR_TYPE_DIRECTORY: {
                Vfs::create(archive->name, Vfs::NodeType::Directory);
                break;
            }
        }

        archive = (Initramfs::UstarHeader*)((uintptr_t)archive + 512 + ALIGN_UP(size, 512)); 
    }
}

}
