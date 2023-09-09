#include <kernel/elf.hpp>
#include <elf.h>

namespace Elf {

bool validate(Elf64_Ehdr header) {
    EXPECT_VALUE(header.e_ident[EI_CLASS], ELFCLASS64)
    EXPECT_VALUE(header.e_ident[EI_DATA], ELFDATA2LSB)
    EXPECT_VALUE(header.e_ident[EI_OSABI], ELFOSABI_SYSV)
    EXPECT_VALUE(header.e_ident[EI_VERSION], EV_CURRENT)
    EXPECT_VALUE(header.e_machine, EM_X86_64)
    EXPECT_VALUE(header.e_version, EV_CURRENT)
    EXPECT_VALUE(header.e_ehsize, sizeof(header))
    return true;
}

bool load(Vmm::AddressSpace* space, Vfs::FileDescriptor* fd, uint64_t load_base, auxval* auxv, 
        char** ld_path) {
    Elf64_Ehdr header;
    if (Vfs::read(fd, &header, sizeof(header)) == 0 || memcmp(header.e_ident, ELFMAG, 4) != 0) {
        return false;
    } else if (!Elf::validate(header)) {
        return false;
    }

    for (size_t i = 0; i < header.e_phnum; i++) {
        Elf64_Phdr phdr;
        Vfs::seek(fd, header.e_phoff + i * header.e_phentsize, Vfs::SeekMode::Set);
        if (Vfs::read(fd, &phdr, sizeof(phdr)) == 0) {
            return false;
        }

        switch (phdr.p_type) {
            case PT_LOAD: {
                int prot = PTE_PRESENT | PTE_USER;
                if (phdr.p_flags & PF_W) {
                    prot |= PTE_WRITABLE;
                }
                if (!(phdr.p_flags & PF_X)) {
                    prot |= PTE_NX;
                }

                size_t misalign = phdr.p_vaddr & (PAGE_SIZE - 1);
                size_t page_count = ALIGN_UP(phdr.p_memsz + misalign, PAGE_SIZE) / PAGE_SIZE;

                void* phys = Pmm::calloc(page_count);
                if (phys == nullptr) {
                    return false;
                }

                space->map_range(phdr.p_vaddr + load_base, (uintptr_t)phys, page_count, prot);

                Vfs::seek(fd, phdr.p_offset, Vfs::SeekMode::Set);
                if (Vfs::read(fd, (void*)PHYS_TO_VIRT((uintptr_t)phys + misalign), phdr.p_filesz) 
                        == 0) {
                    return false;
                }

                break;
            }
            case PT_PHDR:
                auxv->at_phdr = phdr.p_vaddr + load_base;
                break;
            case PT_INTERP: {
                void* path = Heap::kcalloc(phdr.p_filesz + 1);
                if (path == nullptr) {
                    return false;
                }

                Vfs::seek(fd, phdr.p_offset, Vfs::SeekMode::Set);
                if (Vfs::read(fd, path, phdr.p_filesz) == 0) {
                    Heap::kfree(path);
                    return false;
                }

                if (ld_path != NULL) {
                    *ld_path = (char*)path;
                }
                break;
            }
        }
    }

    auxv->at_entry = header.e_entry + load_base;
    auxv->at_phent = header.e_phentsize;
    auxv->at_phnum = header.e_phnum;
    return true;
}

}
