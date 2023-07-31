#include <libc/stdlib.hpp>

int main() {
    while (true) {
        KeyboardEvent event = poll_event();
        if (event.key_code == KeyCode::Invalid) continue;
        putchar(event.ascii);
    }
    return 0;
}
