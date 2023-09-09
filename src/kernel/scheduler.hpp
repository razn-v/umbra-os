#include <kernel/task.hpp>
#include <kernel/int.hpp>

namespace Scheduler {
    
// `TaskList` is a linked list, but with a unique pointer `current` that allows traversal through 
// the nodes of the list. When `next_task` is called, `current_task` moves to the next node in the 
// list, and if no node is there, `current` get assigned to the first element (`front`) of the list.
//
// This design is probably awful, but offers some advantages:
//  - The ability to have as many tasks as possible (no fixed array storing the tasks).
//  - It allows easy traversal of the list with `next_task`, instead of manually dealing with nodes
//  or having a `next` field to the `Task` class.
class TaskList {
public:
    struct Node {
        Task* task;
        Node* prev;
        Node* next;

        Node(Task* task) : task(task), prev(nullptr), next(nullptr) {}
    };

    Node* front;
    Node* current;

    TaskList() : front(nullptr), current(nullptr) {}

    Task* get_current() {
        return this->current->task;
    }

    Task* next_task() {
        this->current = this->current->next;
        if (this->current == nullptr) {
            this->current = this->front;
        }
        return (this->current != nullptr) ? this->current->task : nullptr;
    }

    void add_task(Task* task) {
        Node* node = new Node(task); 
        
        if (this->front == nullptr) {
            this->current = node;
        } else {
            node->next = this->front;
            this->front->prev = node;
        }

        this->front = node;
    }

    void remove_current() {
        if (this->current == nullptr) {
            return;
        }

        if (this->current == this->front) {
            this->front = this->current->next;
        } else {
            this->current->prev->next = this->current->next;
            if (this->current->next != nullptr) {
                this->current->next->prev = this->current->prev;
            }
        }

        Node* next = this->current->next;
        delete this->current;
        if (next != nullptr) {
            this->current = next;
        } else {
            this->current = this->front;
        }
    }

    bool is_empty() {
        return this->front == nullptr;
    }
};

void init();
void schedule(Interrupt::Registers* context);
void yield();
void kill_and_yield();
void add_task(Task* task);
Task* get_current_task();
void add_futex_handle(Task* task);
void remove_futex_handle(Task* task);
void wake_futex_handles(uintptr_t pointer);
void sleep(uint64_t ms);
void await_io();
void wake_io(Keyboard::KeyboardEvent kb_event);

template <typename ConditionFunc>
void block_on(ConditionFunc condition) {
    if (condition()) {
        return;
    }

    while (!condition()) {
        Scheduler::await_io();
    }
}

}
