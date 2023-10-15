#pragma once

#include <kernel/int.hpp>
#include <kernel/memory/vmm.hpp>
#include <kernel/drivers/keyboard.hpp>
#include <kernel/utils/ringbuffer.hpp>
#include <kernel/utils/list.hpp>
#include <kernel/fs/vfs.hpp>
#include <kernel/net/socket.hpp>
#include <stddef.h>

#define TASK_NAME_LENGTH 64

struct auxval {
    uint64_t at_entry;
    uint64_t at_phdr;
    uint64_t at_phent;
    uint64_t at_phnum;
};

struct Task {
    ~Task();

    enum Status {
        Ready,
        Running,
        Sleeping,
        WaitingIo
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
    // File and directory handles
    DoublyLinkedList<Vfs::FileDescriptor*> handles;
    DoublyLinkedList<Socket::Handle*> socket_handles;
    int next_id = 3;
    DoublyLinkedList<uintptr_t> futexes;
    uintptr_t heap_cur;
    char* working_dir;

    static Task* create(const char* name, void (*entry)(), bool user_mode = false, 
            Vmm::AddressSpace* page_map = nullptr, auxval* auxv = nullptr);

    int add_file_handle(Vfs::FileDescriptor* fd);
    int add_socket_handle(Socket::Handle* handle);
    Vfs::FileDescriptor* get_file_handle(int id);
    Socket::Handle* get_socket_handle(int id);
};
