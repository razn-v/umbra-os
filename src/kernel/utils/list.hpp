template <typename T>
class DoublyLinkedList {
private:
    struct Node {
        T value;
        Node* prev;
        Node* next;

        Node(T value) : value(value), prev(nullptr), next(nullptr) {}
    };

    Node* head = nullptr;

public:
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
            this->head->next->prev = node;
            this->head = node;
        }
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
                delete current;
                break;
            }
            current = next;
        }
    }
};
