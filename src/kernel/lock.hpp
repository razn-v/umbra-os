#pragma once

struct Spinlock {
    bool locked = false;
    bool restore_interrupts;

    void acquire();
    void release();
};
