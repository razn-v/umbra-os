#include <limine/limine.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

extern volatile struct limine_rsdp_request rsdp_request;

extern volatile struct limine_memmap_request memmap_request;

extern volatile struct limine_hhdm_request hhdm_request;

extern volatile struct limine_kernel_address_request kernel_address_request;

extern volatile struct limine_module_request module_request;

extern volatile struct limine_boot_time_request time_request;

#define HHDM_OFFSET hhdm_request.response->offset
// The virtual address of the first section of the kernel (.text in our case)
#define VIRTUAL_BASE kernel_address_request.response->virtual_base
#define PHYSICAL_BASE kernel_address_request.response->physical_base
