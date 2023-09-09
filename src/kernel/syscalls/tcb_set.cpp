#include <kernel/syscalls/tcb_set.hpp>
#include <kernel/msr.hpp>
#include <kernel/terminal.hpp>

int sys_tcb_set(void* pointer) {
    Msr::write_msr(Msr::MsrType::Tcb, (uint64_t)pointer);
    return 0;
}
