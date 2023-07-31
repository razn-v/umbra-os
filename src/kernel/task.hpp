#pragma once

#include <kernel/int.hpp>
#include <kernel/memory/vmm.hpp>
#include <kernel/drivers/keyboard.hpp>
#include <kernel/utils/ringbuffer.hpp>
#include <stddef.h>

#define TASK_NAME_LENGTH 64

struct Task {
    ~Task();

    enum Status {
        Ready,
        Running,
        Waiting
    };

    size_t pid;
    Status status;
    Vmm::AddressSpace* address_space;
    Interrupt::Registers* context;
    char name[TASK_NAME_LENGTH];
    // Time stamp count until the task can run again from sleep
    uint64_t tsc_sleep;
    // NOTE: We'll make this a generic ringbuffer once we have other events
    Ringbuffer<Keyboard::KeyboardEvent, 5>* events;

    static Task* create(const char* name, void (*entry)(), bool user_mode = false, 
            Vmm::AddressSpace* page_map = nullptr);
};
