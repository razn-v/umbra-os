#include <stddef.h>
#include <kernel/terminal.hpp>
#include <kernel/utils/optional.hpp>

// TODO: Add destructor
template <typename Value, size_t Capacity>
class StringHashMap {
public:
    struct Entry {
        uint32_t key_hash;
        Value value;
        Entry* next;

        Entry(char* key, Value value) : value(value), next(nullptr) {
            this->key_hash = hash_key(key);
        };
    };

    Entry** entries;
    
    // djb2 algorithm
    static uint32_t hash_key(const char* key) {
        uint32_t hash = 5381;
        int c;

        while ((c = *key++)) {
            hash = ((hash << 5) + hash) ^ c;
        }

        return hash;
    }

    StringHashMap() : entries(new Entry*[Capacity]) {};

    void insert(char* key, Value value) {
        uint32_t hash = hash_key(key);
        uint32_t index = hash % Capacity;

        Entry* entry = this->entries[index];
        Entry* new_entry = new Entry(key, value);

        if (!entry) {
            this->entries[index] = new_entry;
        } else {
            // Iterate until the last entry
            do {
                // Check if the key is already in the hash table
                if (entry->key_hash == hash) {
                    // No need to insert anything
                    return;
                }
                entry = entry->next;
            } while (entry->next != nullptr);

            // Add the entry next to the already existing one
            entry->next = new_entry;
        }
    }

    Optional<Value> get(char* key) {
        uint32_t hash = hash_key(key);
        uint32_t index = hash % Capacity;
        Entry* entry = this->entries[index];

        if (!entry) {
            return Optional<Value>();
        }

        for (; entry != nullptr; entry = entry->next) {
            if (entry->key_hash == hash) {
                return Optional<Value>(entry->value);
            }
        };

        return Optional<Value>();
    }
};
