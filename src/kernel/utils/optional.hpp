#pragma once

template <typename T>
class Optional {
private:
    bool none;
    T value;

public:
    Optional() : none(true) {};
    Optional(T value) : none(false), value(value) {};

    bool is_none() {
        return this->none;
    }

    // TODO Panic if none
    T unwrap() {
        return this->value;
    }
};
