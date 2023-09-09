#include <kernel/memory/vmm.hpp>
#include <kernel/fs/vfs.hpp>
#include <kernel/task.hpp>

#define EXPECT_VALUE(current, expected) if (current != expected) return false;

namespace Elf {

bool load(Vmm::AddressSpace* space, Vfs::FileDescriptor* fd, uint64_t load_base, auxval* auxv, 
        char** ld_path);

}
