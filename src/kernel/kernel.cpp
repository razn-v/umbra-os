#include <kernel/gdt.hpp>
#include <kernel/idt.hpp>
#include <kernel/terminal.hpp>
#include <kernel/apic.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/memory/vmm.hpp>
#include <kernel/memory/heap.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/fs/tmpfs.hpp>
#include <kernel/fs/initramfs.hpp>
#include <kernel/drivers/tty.hpp>
#include <kernel/timer.hpp>
#include <kernel/elf.hpp>
#include <kernel/drivers/pci.hpp>
#include <kernel/drivers/e1000.hpp>

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm volatile("hlt");
    }
}

// TODO: Move this somewhere else
extern "C" void __cxa_pure_virtual() {
    __builtin_unreachable();
}

extern "C" void __cxa_atexit() {
    return;
}

int __dso_handle;

static inline uint64_t read_cr0(void) {
    uint64_t ret;
    asm volatile ("mov %%cr0, %0" : "=r"(ret) :: "memory");
    return ret;
}

static inline void write_cr0(uint64_t value) {
    asm volatile ("mov %0, %%cr0" :: "r"(value) : "memory");
}

static inline uint64_t read_cr4(void) {
    uint64_t ret;
    asm volatile ("mov %%cr4, %0" : "=r"(ret) :: "memory");
    return ret;
}

static inline void write_cr4(uint64_t value) {
    asm volatile ("mov %0, %%cr4" :: "r"(value) : "memory");
}

void kernel_main();

extern "C" void _start(void) {
    // Ensure we got a framebuffer
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer and initialize the terminal
    limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    Terminal::init(framebuffer);

    Terminal::printf("UmbraOS * build %s %s\n", __DATE__, __TIME__);

    Interrupt::init();
    Terminal::printf("{green}[*]{white} Interrupt handlers initialized.\n");

    Gdt::load();
    Terminal::printf("{green}[*]{white} GDT loaded.\n");

    Idt::init();
    Terminal::printf("{green}[*]{white} IDT loaded.\n");

    Timer::init();

    // Initialize local APIC
    Apic::init();
    Terminal::printf("{green}[*]{white} APIC initialized.\n");

    // Initialize the PMM, VMM and the heap
    Pmm::init(memmap_request.response);
    Terminal::printf("{green}[*]{white} PMM initialized.\n");
    Heap::init();
    Terminal::printf("{green}[*]{white} Heap allocator initialized.\n");
    Vmm::init();
    Terminal::printf("{green}[*]{white} VMM initialized.\n");

    Gdt::init_tss();

    Vfs::mount(nullptr, new Tmpfs);
    Vfs::init_std(new Tty);
    Initramfs::init();;

    // Enable SSE/SSE2
    uint64_t cr0 = read_cr0();
    cr0 &= ~((uint64_t)1 << 2);
    cr0 |= (uint64_t)1 << 1;
    write_cr0(cr0);
    uint64_t cr4 = read_cr4();
    cr4 |= (uint64_t)3 << 9;
    write_cr4(cr4);
 
    // We need a task for the kernel because some devices need to sleep to be initialized, which 
    // is only possible inside a task
    auto kernel_task = Task::create("kernel_main", (void(*)())kernel_main);

    // Initialize the scheduler
    Scheduler::init();
    Scheduler::add_task(kernel_task);

    hcf();
}

void kernel_main() {
    Pci::init();

    // Loading the initial ELF
    auxval init_auxv, ld_auxv;
    char* ld_path;
    Vmm::AddressSpace* space = Vmm::new_space();

    auto fd = Vfs::open("/ping", Vfs::OpenMode::ReadOnly);
    Elf::load(space, fd, 0x0, &init_auxv, &ld_path);
    auto ld = Vfs::open(ld_path, Vfs::OpenMode::ReadOnly);
    Elf::load(space, ld, 0x40000000, &ld_auxv, NULL);

    auto elf_test = Task::create("test", (void(*)())ld_auxv.at_entry, true, space, &init_auxv);
    elf_test->heap_cur = init_auxv.at_entry + ALIGN_UP(fd->node->file_size, PAGE_SIZE);
    elf_test->working_dir = strdup("/");

    Vfs::close(fd);
    Vfs::close(ld);

    Scheduler::add_task(elf_test);
    hcf();
}
