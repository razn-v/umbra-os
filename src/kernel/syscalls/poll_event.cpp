#include <kernel/syscalls/poll_event.hpp>

void poll_event(Keyboard::KeyboardEvent* event) {
    Optional<Keyboard::KeyboardEvent> kb_event = Scheduler::get_current_task()->events->read();
    if (kb_event.is_none()) {
        *event = Keyboard::KeyboardEvent(Keyboard::KeyCode::Invalid, '?');
    } else {
        *event = kb_event.unwrap();
    }
}
