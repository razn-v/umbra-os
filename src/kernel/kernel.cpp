#include <kernel/requests.hpp>
#include <stdint.h>
#include <kernel/libc/string.hpp>
#include <kernel/kernel.hpp>
#include <kernel/gdt.hpp>
#include <kernel/idt.hpp>
#include <kernel/int.hpp>
#include <kernel/terminal.hpp>
#include <kernel/apic.hpp>
#include <kernel/io.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/memory/vmm.hpp>
#include <kernel/memory/heap.hpp>
#include <kernel/timer.hpp>
#include <kernel/task.hpp>
#include <kernel/scheduler.hpp>
#include <kernel/lock.hpp>
#include <kernel/fs/vfs.hpp>
#include <kernel/fs/tmpfs.hpp>
#include <kernel/fs/initramfs.hpp>
#include <elf.h>

// TODO remove useless includes

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm volatile("hlt");
    }
}

void wait_func() {
    Terminal::printf("#");
    Scheduler::sleep(10 * 1000);
    Terminal::printf("#");
    Scheduler::kill_and_yield();
}

void wait2_func() {
    while (true) {
        Terminal::printf(".");
        Scheduler::sleep(1 * 1000);
    }
    Scheduler::kill_and_yield();
}

// TODO: Move this somewhere else
// Addresses of the interrupts handlers
extern "C" uintptr_t int_handlers[256];

// TODO: Move this somewhere else
static Tss tss;

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

    // TODO: Why not make this `Gdt::load()`?
    gdt.load();
    Terminal::printf("{green}[*]{white} GDT loaded.\n");

    // TODO: This shouldn't be here
    // Setup each of entry of the IDT
    for (int i = 0; i < 256; i++) {
        if (i == SYSCALL_VECT) {
            // Type: 0b1110 (interrupt gate)
            // Reserved: 0b0
            // DPL: 0b11
            // Present: 0b1
            idt_entries[i] = IdtEntry(int_handlers[i], 0b11101110);
        } else {
            // Type: 0b1110 (interrupt gate)
            // Reserved: 0b0
            // DPL: 0b00
            // Present: 0b1
            idt_entries[i] = IdtEntry(int_handlers[i], 0b10001110);
        }
    }
    // TODO: Make this `Idt::load` too
    idt.load();
    Terminal::printf("{green}[*]{white} IDT loaded.\n");

    // Initialize local APIC
    Apic::init();
    Terminal::printf("{green}[*]{white} APIC initialized.\n");

    // Initialize the PMM, VMM and the heap
    Pmm::init(memmap_request.response);
    Terminal::printf("{green}[*]{white} PMM initialized.\n");
    Vmm::init();
    Terminal::printf("{green}[*]{white} VMM initialized.\n");
    Heap::init();
    Terminal::printf("{green}[*]{white} Heap allocator initialized.\n");

    Vfs::mount('A', new Tmpfs);
    Initramfs::init('A');
    Vfs::print_tree(Vfs::get_mountpoint('A'));

    // Testing if ELFs are working properly
    uint64_t* page_map = Vmm::new_space();
    Elf64_Ehdr header;
    auto fd = Vfs::open('A', "test", Vfs::OpenMode::ReadOnly);
    Vfs::read(fd, &header, sizeof(header));

    Terminal::printf("Entry is at %x\n", header.e_entry);

    for (size_t i = 0; i < header.e_phnum; i++) {
        Elf64_Phdr program_header;
        Vfs::seek_read(fd, header.e_phoff + i * header.e_phentsize, Vfs::SeekMode::Set);
        Vfs::read(fd, &program_header, sizeof(program_header));

        switch (program_header.p_type) {
            case PT_NULL: {
                break;
            }
            case PT_LOAD: {
                // FIXME: Do we have to account for misalignment?
                size_t pages = ALIGN_UP(program_header.p_memsz, PAGE_SIZE) / PAGE_SIZE;
                void* phys = Pmm::calloc(pages);

                // FIXME This shouldn't be RW or executable in all cases
                Vmm::map_range(page_map, program_header.p_vaddr, (uintptr_t)phys, pages,
                        PTE_PRESENT | PTE_WRITABLE | PTE_USER);
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

    auto elf_test = Task::create("test", (void(*)())header.e_entry, true, page_map);

    // TODO: This shouldn't be here
    memset(&tss, 0, sizeof(Tss)); 
    tss.rsp[0] = PHYS_TO_VIRT((uintptr_t)Pmm::alloc(STACK_SIZE / PAGE_SIZE) + STACK_SIZE);
    init_tss(&tss);

    // Initialize the scheduler
    Scheduler::init();
    Scheduler::add_task(elf_test);

    hcf();
}
