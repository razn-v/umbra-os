#include <stddef.h>
#include <stdint.h>
#include <kernel/utils/optional.hpp>

// TODO: Add destructor
template <typename Value, size_t Capacity>
class StringHashMap {
private:
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

public:
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
            while (entry->next != nullptr) {
                // Check if the key is already in the hash table
                if (entry->key_hash == hash) {
                    // No need to insert anything
                    return;
                }
                entry = entry->next;
            }

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

    // Not optimal at all. We should refactor this whole thing tbh.
    Optional<Value> get_nth(size_t n) {
        size_t count = 0;

        for (size_t i = 0; i < Capacity; ++i) {
            Entry* entry = entries[i];
            while (entry != nullptr) {
                // Check if the entry is not empty
                if (entry->key_hash != 0) {
                    if (count == n) {
                        return Optional<Value>(entry->value);
                    }
                    ++count;
                }
                entry = entry->next;
            }
        }

        return Optional<Value>();
    }
};
