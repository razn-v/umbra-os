#include <kernel/memory/pmm.hpp>
#include <kernel/libc/misc.hpp>
#include <kernel/terminal.hpp>
#include <kernel/utils/bitmap.hpp>
#include <kernel/libc/string.hpp>
#include <kernel/lock.hpp>

namespace Pmm {

static Spinlock pmm_lock;

static Bitmap bitmap;
static size_t last_page_idx = 0;

void init(struct limine_memmap_response* memmap) {
    // Calculate the size of the bitmap by finding the highest address in the memory maps provided
    // the bootloader
    uint64_t highest_addr = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            highest_addr = MAX(highest_addr, entry->base + entry->length);
        }
    }

    uint64_t number_of_pages = highest_addr / PAGE_SIZE;
    // Size of the bitmap in bytes
    bitmap.size = ALIGN_UP(number_of_pages / 8, PAGE_SIZE);

    // Find a place to put the bitmap
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap.size) {
            // We found a place to put our bitmap. The address of the entry is physical, so we need
            // to convert it to a virtual address first with the help of the higher-half map
            // provided by limine while paging is enabled.
            bitmap.data = (uint8_t*)PHYS_TO_VIRT(entry->base);

            // Mark all of the pages as used (1)
            memset(bitmap.data, 0xff, bitmap.size);

            // Make the memory region used by the bitmap unusable
            entry->length -= bitmap.size;
            entry->base += bitmap.size;
            break;
       }
    }

    // Find free pages and populate the bitmap properly
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;

        // Free each page of the entry
        for (size_t j = 0; j < entry->length; j += PAGE_SIZE) {
            bitmap.set((entry->base + j) / PAGE_SIZE, false);
        }
    }
}


// NOTE: Returns a physical address
void* inner_alloc(size_t pages, uint64_t limit) {
    size_t acc = 0;

    // Find the first range in the bitmap that meet the requirements
    while (last_page_idx < limit) {
        if (!bitmap.get(last_page_idx++)) {
            if (++acc == pages) {
                // Mark all pages as used
                size_t page = last_page_idx - pages;
                for (size_t i = page; i < last_page_idx; i++) {
                    bitmap.set(i, true);
                }
                return (void*)(page * PAGE_SIZE);
            }
        } else {
            acc = 0;
        }
    }

    return nullptr;
}

void* alloc(size_t pages) {
    if (pages == 0) return nullptr;

    pmm_lock.acquire();
    void* addr = Pmm::inner_alloc(pages, bitmap.size * PAGE_SIZE);

    // If we didn't find any free page, we go back searching for one from the beginning of the 
    // bitmap to the last bit used.
    if (addr == nullptr) {
        size_t last = last_page_idx;
        last_page_idx = 0;
        addr = Pmm::inner_alloc(pages, last);
    }

    pmm_lock.release();
    return addr;
}

void* calloc(size_t pages) {
    if (pages == 0) return nullptr;

    void* ret = Pmm::alloc(pages);
    if (ret != nullptr) {
        memset((void*)PHYS_TO_VIRT((uintptr_t)ret), 0, pages * PAGE_SIZE);
    }
    return ret;
}

void free(void* addr, size_t pages) {
    pmm_lock.acquire();

    size_t page_idx = (uint64_t)addr / PAGE_SIZE;
    for (size_t i = page_idx; i < page_idx + pages; i++) {
        bitmap.set(i, false);
    }

    pmm_lock.release();
}

}
