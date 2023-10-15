#pragma once

#include <kernel/utils/optional.hpp>

// TODO: Add an iterator
template <typename T>
class DoublyLinkedList {
public:
    struct Node {
        T value;
        Node* prev;
        Node* next;

        Node(T value) : value(value), prev(nullptr), next(nullptr) {}
    };

    Node* head = nullptr;
    size_t size = 0;

    ~DoublyLinkedList() {
        Node* current = this->head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    void push(T value) {
        Node* node = new Node(value);

        if (this->head == nullptr) {
            this->head = node;
        } else {
            node->next = this->head;
            if (this->head->next != nullptr) this->head->next->prev = node;
            this->head = node;
        }

        this->size++;
    }

    void remove(T value) {
        Node* current = this->head;

        while (current != nullptr) {
            Node* next = current->next;
            if (current->value == value) {
                if (current == this->head) {
                    this->head = this->head->next;
                    if (this->head != nullptr) {
                        this->head->prev = nullptr;
                    }
                } else {
                    current->prev->next = current->next;
                    if (current->next != nullptr) {
                        current->next->prev = current->prev;
                    }
                }
                this->size--;
                delete current;
                break;
            }
            current = next;
        }
    }

    Optional<T> get(T value) {
        Node* current = this->head;
        while (current != nullptr) {
            if (current->value == value) {
                return Optional<T>(value);
            }
            current = current->next;
        }
        return Optional<T>();
    }

    size_t get_size() {
        return this->size;
    }

    bool is_empty() {
        return this->head == nullptr;
    }
};
