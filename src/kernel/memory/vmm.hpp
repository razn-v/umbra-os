#include <stdint.h>
#include <stddef.h>

struct ElfSymbol {
    uintptr_t address;
    char* name;
};

extern const ElfSymbol text_start, text_end;
extern const ElfSymbol rodata_start, rodata_end;
extern const ElfSymbol data_start, data_end;

#define PTE_PRESENT (1ull << 0ull)
#define PTE_WRITABLE (1ull << 1ull)
#define PTE_USER (1ull << 2ull)
#define PTE_NX (1ull << 63ull)
#define PTE_ADDR_MASK 0x000ffffffffff000
#define PTE_GET_ADDR(VALUE) ((VALUE) & PTE_ADDR_MASK)

namespace Vmm {

void init();

void map(uint64_t* pml4, uintptr_t virt_addr, uintptr_t phys_addr, uint64_t flags);
void map_range(uint64_t* pml4, uintptr_t virt_addr, uintptr_t phys_addr, size_t pages, 
        uint64_t flags); 

void switch_space(uint64_t* pml4);
uint64_t* new_space();

}
