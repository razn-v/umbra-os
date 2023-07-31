#include <limine/limine.h>
#include <kernel/requests.hpp>
#include <stddef.h>
#include <kernel/libc/string.hpp>

#define PAGE_SIZE (4 * 1024)
#define STACK_SIZE (1024 * 1024)
// One page below the maximum address in the user address space
#define USER_STACK_BASE 0x00007ffffffff000

#define ALIGN_UP(addr, align) (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))

#define PHYS_TO_VIRT(addr) (addr + HHDM_OFFSET)
#define VIRT_TO_PHYS(addr) (addr - HHDM_OFFSET)
#define IO_TO_PHYS(addr) (addr - VIRTUAL_BASE + PHYSICAL_BASE)

namespace Pmm {

void init(struct limine_memmap_response*);

void* alloc(size_t pages);
void* calloc(size_t pages);
void free(void* addr, size_t pages);

}
