#include <kernel/syscalls/log.hpp>
#include <kernel/syscalls/mmap.hpp>
#include <kernel/syscalls/tcb_set.hpp>
#include <kernel/syscalls/exit.hpp>
#include <kernel/syscalls/futex.hpp>
#include <kernel/syscalls/fs.hpp>
#include <kernel/syscalls/gui.hpp>
#include <kernel/syscalls/clock.hpp>
#include <libs/core/syscalls.hpp>
#include <kernel/int.hpp>

namespace Syscall {

void handler(Interrupt::Registers* regs);

}
