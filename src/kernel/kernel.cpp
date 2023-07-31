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
#include <elf.h>

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

    Vfs::mount('A', new Tmpfs);
    Initramfs::init('A');
    Vfs::print_tree(Vfs::get_mountpoint('A'));

    // Testing if ELFs are working properly
    Vmm::AddressSpace* space = Vmm::new_space();
    Elf64_Ehdr header;
    auto fd = Vfs::open('A', "test", Vfs::OpenMode::ReadOnly);
    Vfs::read(fd, &header, sizeof(header));

    Terminal::printf("Entry is at %x\n", header.e_entry);

    for (size_t i = 0; i < header.e_phnum; i++) {
        Elf64_Phdr program_header;
        Vfs::seek_read(fd, header.e_phoff + i * header.e_phentsize, Vfs::SeekMode::Set);
        Vfs::read(fd, &program_header, sizeof(program_header));

        switch (program_header.p_type) {
            case PT_LOAD: {
                // FIXME: Do we have to account for misalignment?
                size_t pages = ALIGN_UP(program_header.p_memsz, PAGE_SIZE) / PAGE_SIZE;
                void* phys = Pmm::calloc(pages);

                // FIXME This shouldn't be RW or executable in all cases
                space->map_range(program_header.p_vaddr, (uintptr_t)phys, pages, PTE_PRESENT | 
                        PTE_WRITABLE | PTE_USER);
                void* virt = (void*)PHYS_TO_VIRT((uintptr_t)phys);

                Vfs::seek_read(fd, program_header.p_offset, Vfs::SeekMode::Set);
                Vfs::read(fd, virt, program_header.p_filesz);
                break;
            }
            default: {
                break;
            }
        }
    }

    Vfs::close(fd);

    auto elf_test = Task::create("test", (void(*)())header.e_entry, true, space);

    // Initialize the scheduler
    Scheduler::init();
    Scheduler::add_task(elf_test);

    hcf();
}
