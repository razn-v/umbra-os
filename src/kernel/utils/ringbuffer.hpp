#pragma once

#include <kernel/utils/optional.hpp>

template <typename T, size_t Size>
class Ringbuffer {
private:
    T* data;
    size_t head;
    size_t tail;

public:
    Ringbuffer() : data(new T[Size]), head(0), tail(0) {}

    ~Ringbuffer() {
        delete[] this->data;
    }

    void write(T item) {
        // If the buffer is full, the new item will overwrite the oldest one
        if ((head + 1) % Size == tail) {
            tail = (tail + 1) % Size;
        }

        data[head] = item;
        head = (head + 1) % Size;
    }

    Optional<T> read() {
        // If the buffer is empty
        if (head == tail) {
            return Optional<T>();
        }

        T item = data[tail];
        tail = (tail + 1) % Size;
        return Optional<T>(item);
    }

    bool is_empty() {
        return head == tail;
    }
};
