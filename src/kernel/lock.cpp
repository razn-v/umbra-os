#include <kernel/lock.hpp>
#include <stddef.h>
#include <kernel/int.hpp>
#include <kernel/apic.hpp>

void Spinlock::acquire() {
    this->restore_interrupts = Interrupt::interrupts_enabled();
    // To prevent deadlocks, we disable interrupts before attempting to acquire the spinlock.
    // This means that a non-running task will never hold a lock. Interrupts are re-enabled after 
    // the lock is released.
    Interrupt::disable_interrupts();

    while (__atomic_test_and_set(&this->locked, __ATOMIC_SEQ_CST)) {
        asm volatile("pause");
    }
}

void Spinlock::release() {
    __atomic_store_n(&this->locked, false, __ATOMIC_SEQ_CST);

    // We have to check if interrupts were initially disabled before acquiring the lock
    if (this->restore_interrupts) {
        Interrupt::enable_interrupts();
    }
}
